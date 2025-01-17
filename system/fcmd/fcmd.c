/*******************************************************************************
--函数命令行V0.03
--fcmd.c
--周瑞鹏
--2015-3-5
--
--note:
	该模块主要用于调试用途，可以方便的进行手动测试，向调用C语言函数一样在运行阶段执行
	想要执行的C函数。
	比如:调试单片机的PWM时，执行int timer_pwm_set(int pwm)
          输入timer_pwm_set(88)，就可以改变占空比了
--history:
	0.01运行正常，只能支持int32_t类型参数
	0.02新增加字符串类型的支持,eg:disp(50,-60, "ADC:  mV", 10, "89"),
		调换了CmdTbl内部成员的顺序，字符串在前，函数指针在后
		增加了内存显示命令,c库函数都可以正常调用测试
		目前运行正常
	0.03增加对可变参数的支持,eg:int printf(const char *fmt, ...)
*******************************************************************************/
#include "fcmd.h"


#define PARAMS_NUM  10 //函数支持10个参数
#define SYSTEM_CMD  3  //系统命令的个数, CmdTbl增加系统命令的同时也要增加这个变量

#define _args_t  int	//参数类型，对于16bit和8bit单片机要注意

/*******************************************************************************
 * 一个函数命令行结构
 */
typedef struct
{
    char *fname;
    void (*pfunc)(void);
} CmdTbl_t;

#include "fcmd_cfg.h"


/*******************************************************************************
 * 静态函数声明
 */
static void system_cmd_exe(uint8_t n);
static int8_t get_args_num(uint8_t *str, uint8_t *key);
static void get_args(uint8_t *str, uint8_t *key, _args_t *args);
static uint8_t *get_fname(uint8_t *str, uint8_t *len);

/*
 * 执行模块定义的指令
 * @n 指令序号，指令CmdTbl中的序号
 */
static void system_cmd_exe(uint8_t n)
{
    uint8_t i;

    switch (n)
    {
    case 0://ls:列出所有的命令
        PRINTF("------------- function list --------------\n");
        for (i = 0; i < CmdTblSize; i++)
        {
            PRINTF("%s\n", CmdTbl[i].fname);
        }
        PRINTF("-------------------------------------------\n");
        break;

    case 1://addr:列出所有函数指针的地址
        PRINTF("------------------------------------------\n");
        for (i = 0; i < CmdTblSize; i++)
        {
            PRINTF("0x%08x: %s\n", (int)CmdTbl[i].pfunc, CmdTbl[i].fname);
        }
        PRINTF("------------------------------------------\n");
        break;

    case 2:
        PRINTF(
            "---------------------------------------------\n"
            "fcmd V0.03  zrpeng\n"
            "Used:\n"
            "  do_mem_md(0x00141280, 512, 1)\n"
            "  memset(0x00141280, 65, 512)\n"
            "  malloc(1024)\n"
            "  free(0x00141280)\n"
            "---------------------------------------------\n"
        );
        break;
    }
}

/*
 * 获取函数的参数个数
 * @str 函数命令字符串
 * @key 参数分隔符，eg: (,) 字符串内部的,不算 eg:"auther:zrp,2015"
 *
 * return 参数的个数
 *        -2   可变参数(参数个数不定)
 *        -1   一个分隔符也没有，eg：ls命令就没有()分隔符
 *        >=0  参数个数，eg:timer_pwm_set(5,67)，分隔符是(,)很明显有2个参数
 */
static int8_t get_args_num(uint8_t *str, uint8_t *key)
{
    uint8_t *pch;
    uint8_t *pbrk[PARAMS_NUM * 2]; //指向断点(,)
    int8_t brk_cnt;

    brk_cnt = 0;
    pch = (uint8_t *)strpbrk ((char *)str, (char *)key);
    while (pch != NULL)
    {
        pbrk[brk_cnt] = pch;
        brk_cnt++;
        pch = (uint8_t *)strpbrk ((char *)(pch + 1), (char *)key);
    }

    if (brk_cnt == 2)//"(void)" or "(int a)" or "(  )" or "(void *p)"
    {
        uint8_t *t = pbrk[0] + 1;

        if (strstr((char *)(pbrk[0] + 1), "void") != NULL)
        {
            if (strchr((char *)(pbrk[0] + 1), '*') != NULL)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }

        while (t != pbrk[1])//(  )
        {
            if (*t == ' ')
            {
                t++;
            }
            else
            {
                return 1;
            }
        }

        return 0;
    }
    else if (brk_cnt == 3)
    {
        //判断可变参数的情况,eg:int printf(const char *fmt, ...)
        if (strstr((char *)pbrk[0], ".." ) != NULL)
        {
            return -2;
        }
    }

    return brk_cnt - 1;
}

/*
 * 判断字符串区间是否为空, \r \n \f \v \t ' '
 * @head  区间头指针
 * @tail  区间尾部
 * return 为空返回TRUE, 不为空返回FALSE
 * note   [head, tail), 左闭右开
 */
static int8_t span_isspace(uint8_t *head, uint8_t *tail)
{
    while (head != tail)
    {
        if (isspace(*head))
        {
            head++;
        }
        else
        {
            return 0;
        }
    }
    return 1;
}

/*
 * 分离参数，支持int32_t和字符串, eg:disp(10,-60, "string", 10, "sub:(a,b)")
 * @str 函数字符串
 * @key 分隔符字符串，eg: (,)"
 *
 * note  字符串参数不支持转义字符, 直接输入"'\即可识别, eg:"'\r直接识别成4个字符.
 *       合法字符串示范:disp("value:"56V,""), disp("value:\"56\""),
 *       要保证"成对出现,否则报错
 */
static void get_args(uint8_t *str, uint8_t *key, _args_t *args)
{
    uint8_t *pch;
    uint8_t *pbrk[PARAMS_NUM * 3]; //指向分隔符(,)"
    int8_t brk_cnt;

    brk_cnt = 0;
    pch = (uint8_t *)strpbrk ((char *)str, (char *)key);
    while ((pch != NULL) && (brk_cnt < PARAMS_NUM * 3))	//分隔符不能太多
    {
        pbrk[brk_cnt] = pch;
        brk_cnt++;
        pch = (uint8_t *)strpbrk ((char *)(pch + 1), (char *)key);
    }

    if (brk_cnt == 0)//没有(,)就是系统命令
    {
        args[0] = -1;
    }
    else if (brk_cnt == 1)//只有有一个分隔符说明输入错误
    {
        args[0] = -2;//函数错误
    }
    else//2个及以上
    {
        if (*pbrk[0] == '(' && *pbrk[brk_cnt - 1] == ')')
        {
            args[0] = 0;

            if (brk_cnt == 2)//2个，两种可能：一个参数或没有参数
            {
                uint8_t *t = pbrk[0] + 1;

                while (t != pbrk[1])
                {
                    if (*t == ' ')
                    {
                        t++;
                        args[0] = 0;
                    }
                    else
                    {
                        args[0] = 1;//有一个参数
                        args[1] = strtol((char *)(pbrk[0] + 1), NULL, 0);
                        break;
                    }
                }
            }
            else//3个及以上, 处理最复杂的情况
            {
                uint8_t quotation_cnt;
                uint8_t i;
                uint8_t args_cnt;
                uint8_t state;
                uint8_t finding;

                //双引号数量是否成对
                quotation_cnt = 0;
                for (i = 1; i < brk_cnt; i++)
                {
                    if (*pbrk[i] == '\"')
                    {
                        quotation_cnt++;
                    }
                }

                if (quotation_cnt == 0)//没有字符串参数
                {
                    for (i = 0; i < brk_cnt; i++)
                    {
                        *pbrk[i] = '\0';    //替换成字符串结束符
                    }

                    for (i = 0; i < brk_cnt; i++)//转换成整型数
                    {
                        args[i + 1] = strtol((char *)(pbrk[i] + 1), NULL, 0);
                    }

                    args[0] = brk_cnt - 1;

                    return ;
                }
                else if ((quotation_cnt % 2) != 0) //没有成对出现"
                {
                    args[0] = -2;//函数错误
                    return ;
                }

                //参数转换, 此时分隔符数量>=3
                args_cnt = 1;
                state = 0;
                i = 0;
                while (i < brk_cnt - 1)	//少一个, 结尾的')'不考虑
                {
                    finding = 1;
                    while (finding)
                    {
                        switch (state)
                        {
                        case 0://识别",
                            {
                                if (i == 0)
                                {
                                    if (*pbrk[i + 1] == '\"')
                                    {
                                        i++;
                                        state = 2;
                                    }
                                    else if (*pbrk[i + 1] == ',')
                                    {
                                        state = 1;
                                    }
                                }
                                else if (*pbrk[i] == ',')
                                {
                                    if (*pbrk[i + 1] == ',' || *pbrk[i + 1] == ')')
                                    {
                                        state = 1;
                                    }
                                    else if (*pbrk[i + 1] == '\"')
                                    {
                                        i++;
                                        state = 2;
                                    }
                                }
                            }
                            break;

                        case 1://整型参数
                            {
                                args[args_cnt] = strtol((char *)(pbrk[i] + 1), NULL, 0);
                                args_cnt++;

                                i++;			//下一个,
                                finding = 0;	//跳出循环，判断一下此时i的范围
                                state = 0;
                            }
                            break;

                        case 2://字符串参数
                            {
                                uint8_t is_string_end;

                                args[args_cnt] = (_args_t)(pbrk[i] + 1);
                                args_cnt++;

                                //找出",即可判断字符串结束了
                                is_string_end = 0;
                                while (!is_string_end)	//找出字符串结尾
                                {
                                    i++;
                                    while (*pbrk[i] != '\"')//找出配对的"
                                    {
                                        i++;
                                    }

                                    i++;		//下一个,
                                    if (i >= brk_cnt - 1)
                                    {
                                        is_string_end = 1;
                                        *pbrk[i - 1] = 0;		//字符串尾部"替换成结束符
                                        break;				//$! 到命令行结尾了,退出整个函数
                                    }

                                    if (*pbrk[i] == ',')	//到字符串尾部了?
                                    {
                                        //进一步判断"和,之间是不是空字符
                                        if (span_isspace(pbrk[i - 1] + 1, pbrk[i]))
                                        {
                                            is_string_end = 1;//$! 确实是字符串尾部
                                            *pbrk[i - 1] = 0;	// 字符串尾部"替换成结束符
                                            break;
                                        }
                                        else
                                        {
                                            //"  """" "和,之间有字符
                                            i++;
                                            while (*pbrk[i] != '\"')//找出第一个"
                                            {
                                                i++;
                                            }
                                        }
                                    }
                                    else if (*pbrk[i] == '\"')	//又是第一个"
                                    {}
                                }

                                finding = 0;	//跳出循环，判断一下此时i的范围
                                state = 0;
                            }
                            break;
                        }
                    }
                }

                args[0] = args_cnt - 1;
            }

            *pbrk[0] = 0;	//'('清掉，保证函数名结尾为0
        }
        else
        {
            args[0] = -2;//格式错误
        }
    }
}

/*
 * 从CmdTbl中获取函数名
 * @str CmdTbl[i].fname
 * @len 返回函数名长度
 *
 * return 返回用户函数名指针
 */
static uint8_t *get_fname(uint8_t *str, uint8_t *len)
{
    uint8_t *phead;
    uint8_t *pend;

    phead = (uint8_t *)strchr((char *)str, '(');

    //跳过尾部空格,\t
    phead--;
    while (*phead == ' ' || *phead == '\t')
    {
        phead--;
    }
    pend = phead;

    //找到头部的 ' ' 或者 '*'
    while (*phead != ' ' && *phead != '*' && *phead != '\t')
    {
        phead--;
    }
    phead++;

    *len = pend - phead + 1;
    return phead;
}
/*
 * 函数命令行执行
 * @cmd 函数命令行, eg: timer_pwm_set(0, 567)
 */
void fcmd_exec(uint8_t *cmd)
{
    uint8_t *pcmd = cmd;
    _args_t args[PARAMS_NUM + 1]; 	//参数数组
    _args_t ret = 0;					//函数返回值
    uint8_t i;
    int8_t cmdtbl_param_num;

    //跳过开头空格
    while (*pcmd == ' ')
    {
        pcmd++;
    }

    //分离参数
    memset(args, 0, PARAMS_NUM + 1);
    get_args(pcmd, (uint8_t *)"(,)\"", args);

    if (args[0] == -2)
    {
        PRINTF("err:fmt err\n");
        return ;
    }

    //匹配命令
    if (args[0] == -1)
    {
        //系统命令
        for (i = 0; i < SYSTEM_CMD; i++)
        {
            if (strncmp((char *)pcmd, CmdTbl[i].fname, strlen((char *)pcmd)) == 0)
            {
                break;
            }
        }

        if (i >= SYSTEM_CMD)
        {
            PRINTF("err:system cmd err\n");
            return ;
        }
    }
    else
    {
        //普通命令
        for (i = SYSTEM_CMD; i < CmdTblSize; i++)
        {
            uint8_t *pcmd_end;
            uint8_t pfname_len;
            uint8_t pcmd_len;
            uint8_t *pfname = get_fname((uint8_t *)CmdTbl[i].fname, &pfname_len);

            //malloc (int size), pcmd_end指向'c'
            pcmd_end = pcmd + strlen((char *)pcmd) - 1;
            while (*pcmd_end == ' ')//跳过空格
            {
                --pcmd_end;
            }
            pcmd_len = pcmd_end - pcmd + 1;

            //比较函数名
            if (strncmp((char *)pfname, (char *)pcmd, pcmd_len > pfname_len ? pcmd_len : pfname_len) == 0)
            {
                break;
            }
        }

        //没有匹配到命令
        if (i >= CmdTblSize)
        {
            PRINTF("err:not you wanted cmd\n");
            return;
        }
    }

    //得到函数表里的函数的参数个数
    if (args[0] != -1)
    {
        //普通命令
        cmdtbl_param_num = get_args_num((uint8_t *)CmdTbl[i].fname, (uint8_t *)"(,)");
        if (cmdtbl_param_num == -2)
        {
            ;//可变参数情况
        }
        else if (cmdtbl_param_num != args[0])
        {
            PRINTF("err:param err\n");
            return ;
        }
    }
    else
    {
        //执行系统命令
        system_cmd_exe(i);
        return;
    }

    //传入相应参数，执行普通命令
    switch (args[0])
    {
    case 0://没有参数
        ret = (*(_args_t(*)(void))CmdTbl[i].pfunc)();
        break;

    case 1:
        ret = (*(_args_t(*)(_args_t))CmdTbl[i].pfunc)(args[1]);
        break;

    case 2:
        ret = (*(_args_t(*)(_args_t, _args_t))CmdTbl[i].pfunc)(args[1], args[2]);
        break;

    case 3:
        ret = (*(_args_t(*)(_args_t, _args_t, _args_t))CmdTbl[i].pfunc)(args[1], args[2], args[3]);
        break;

    case 4:
        ret = (*(_args_t(*)(_args_t, _args_t, _args_t, _args_t))CmdTbl[i].pfunc)(args[1], args[2], args[3], args[4]);
        break;

    case 5:
        ret = (*(_args_t(*)(_args_t, _args_t, _args_t, _args_t, _args_t))CmdTbl[i].pfunc)(args[1], args[2], args[3], args[4], args[5]);
        break;

    case 6:
        ret = (*(_args_t(*)(_args_t, _args_t, _args_t, _args_t, _args_t, _args_t))CmdTbl[i].pfunc)(args[1], args[2], args[3], args[4], args[5], args[6]);
        break;

    case 7:
        ret = (*(_args_t(*)(_args_t, _args_t, _args_t, _args_t, _args_t, _args_t, _args_t))CmdTbl[i].pfunc)(args[1], args[2], args[3], args[4], args[5], args[6],
                args[7]);
        break;

    case 8:
        ret = (*(_args_t(*)(_args_t, _args_t, _args_t, _args_t, _args_t, _args_t, _args_t, _args_t))CmdTbl[i].pfunc)(args[1], args[2], args[3], args[4], args[5],
                args[6], args[7], args[8]);
        break;

    case 9:
        ret = (*(_args_t(*)(_args_t, _args_t, _args_t, _args_t, _args_t, _args_t, _args_t, _args_t, _args_t))CmdTbl[i].pfunc)(args[1], args[2], args[3], args[4],
                args[5], args[6], args[7], args[8], args[9]);
        break;

    case 10:
        ret = (*(_args_t(*)(_args_t, _args_t, _args_t, _args_t, _args_t, _args_t, _args_t, _args_t, _args_t, _args_t))CmdTbl[i].pfunc)(args[1], args[2], args[3],
                args[4], args[5], args[6], args[7], args[8], args[9], args[10]);
        break;

    default:
        PRINTF("err:params num err\n");
        break;
    }

    if (sizeof(_args_t) == 1)
    {
        PRINTF("=0x%02x,%d;\n", ret, ret);
    }
    else if (sizeof(_args_t) == 2)
    {
        PRINTF("=0x%04x,%d;\n", ret, ret);
    }
    else if (sizeof(_args_t) == 4)
    {
        PRINTF("=0x%08x,%d;\n", ret, ret);
    }
}


