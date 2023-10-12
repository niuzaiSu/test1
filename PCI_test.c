#ifdef _WIN32   // Windows系统下的代码

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// #include <windows.h>
#include <time.h>
#include <unistd.h>

#define CMD_RESULT_BUF_SIZE 1024

char result[CMD_RESULT_BUF_SIZE]={0};
char debug_log[CMD_RESULT_BUF_SIZE]={0};

void Compliance(char * test_items, const char * device);/* PCI合规检测 */
int compareVersions(char* version1, char* version2) ;/* 比较版本号新旧 */
int ExecuteCMD(const char* cmd, char* result_);

#if 1
int main(int argc, char *argv[])
{
	// printf("argc = %d\n", argc);
     // for(int i=0; i<argc; ++i)
    // printf("%s\n", argv[i]);	
        
    // char result1[CMD_RESULT_BUF_SIZE]={0};
    // if("设备已连接再检测")

    if (argc < 2) {
        printf("Usage: %s <command>\n", argv[0]);
        return 1;
    }
    
    /* PCI_test.exe --test "24hour|ASLR|DEP|RELRO|Application_isolation|APK" --device P201221720021 --key @sunmi.com */    
    /* compliance.exe --test "24hour|Application_isolation" --device P201221720021 --key @sunmi.com */
    char* device = NULL;
    char* key = NULL;

    if(argv[6] != NULL){
        key = argv[6];
    }

    if(argv[4] != NULL){
        device = argv[4];
    }
    // else{
    //     ExecuteCMD("adb devices", result);
    //     printf("Usage: %s <command>\n");
    //     memset(result, 0, sizeof(result));
    // }

    /* 使用 strtok 函数按照 '|' 切分命令参数 */
    char* cmd = strtok(argv[2], "|");
    // char* cmd1 = cmd;
    while (cmd != NULL) {
        // printf("%s\n", cmd);
        if(strcmp(key, "@sunmi.com") == 0){
            Compliance(cmd, device);
        }else{
            printf("wrong password\n");
        }

        // for(int i=0; i<argc; ++i){
        //     Compliance(cmd, device);
        // }    

        // 继续比较下一位
        cmd = strtok(NULL, "|");
    }

    // printf("%d\n", __LINE__); 
    // printf("%s\n%s\n%s\n", result1, result2, result3);
    // printf("%s\n%s\n", result, cmd1);
    printf("%s\n", result);

    system("pause");    // 防止运行后自动退出，需头文件stdlib.h

    return 0;
}
#endif

/* PCI合规检测 */
void Compliance(char * test_items, const char * device)
{
    char* cmd = NULL;
    char* sel = NULL;
    sel = strstr(device, "5555");
    // printf(device);
    // printf(sel);

    /* 设备识别 */
    // if(strcmp(device, "P201221720021") == 0){
    /* PCI合规检测项 */
        /* 24小时重启 */
        if(strcmp(test_items, "24hour")==0){
            char ret_boot_time[CMD_RESULT_BUF_SIZE]={0};

            if (sel != NULL){                
                sprintf(cmd, "adb -s %s shell cat /proc/stat | find \"btime\"", device);
                printf(cmd);
                ExecuteCMD(cmd, ret_boot_time);
            }else{   
                printf(cmd);             
                ExecuteCMD("adb shell cat /proc/stat | find \"btime\"", ret_boot_time); //获取设备开机时间-验证24小时重启
            }

            {
                // 获取当前时间戳
                time_t current_time = time(NULL);   // printf("%d\n", current_time);
                // 获取设备开机时间戳
                time_t num1 = atoll((const char *)ret_boot_time);
                time_t boot_time = num1; // 这里替换成实际获取到的设备开机时间戳

                // 计算时间差值（以秒为单位）
                double time_diff = difftime(current_time, boot_time);
                static _Bool  flag=0;
                static time_t boot_time_old;

                // 判断时间差是否接近24小时（容许误差范围为60秒）
                // if (time_diff > 86400) {
                //     // strcat(result, "设备具备24小时自动重启功能!\n");  //char *strcat(char *dest, const char *src) 把 src 所指向的字符串追加到 dest 所指向的字符串的结尾。
                //     strcat(result, "success,0\n");
                //     char * temp = NULL; 
                //     sprintf(temp, "%d", time_diff);
                //     strcat(result, temp);
                // } 
                // else {
                //     if(flag==0){
                //         strcat(result, "请确保设备手动开机24小时后重新检测\n");
                //         boot_time_old = boot_time;
                //         flag=1;
                //     }
                //     else if(flag==1 && boot_time_old != boot_time){
                //         strcat(result, "success,1\n");
                //         // boot_time_old = boot_time;
                //     }
                //     else{
                //         strcat(result, "请确保设备手动开机24小时后重新检测\n");
                //         flag=0;
                //     }                        
                // }                 

                if (time_diff >= 86400 - 60 && time_diff <= 86400 + 60) {
                    // strcat(result, "设备具备24小时自动重启功能!\n");
                    strcat(result, "success,1\n"); 
                } else {
                    // strcat(result, "设备没有24小时自动重启功能。\n");
                    strcat(result, "success,0\n");
                }
            }
        }
        /* 启用ASLR */
        else if(strcmp(test_items, "ASLR")==0){
            char ret_ASLR[CMD_RESULT_BUF_SIZE]={0};

            if (sel != NULL){
                sprintf(cmd, "adb -s %s shell cat /proc/sys/kernel/randomize_va_space", device);
                ExecuteCMD(cmd, ret_ASLR);
            }else{                
                ExecuteCMD("adb shell cat /proc/sys/kernel/randomize_va_space", ret_ASLR);    //查询设备是否启用ASLR
            }           
            
            // printf(ret_ASLR);
            char* pch = NULL;
            // pch = strstr(ret_ASLR, "2");
            pch = strchr(ret_ASLR, '2');
            if (pch != NULL){
                // strcat(result, "success, 已启用ASLR\n");
                strcat(result, "success,1\n");
            }else{
                memset(ret_ASLR, 0, sizeof(ret_ASLR));
                ExecuteCMD("adb root", ret_ASLR);
                char* canRoot = NULL;
                canRoot = strstr(ret_ASLR, "cannot");
            
                if (canRoot != NULL){
                    strcat(result, "adbd cannot run as root in production builds\n");
                    memset(ret_ASLR, 0, sizeof(ret_ASLR));
                }else{
                    // strcat(result, "success, 未启用ASLR\n");
                    strcat(result, "success,0\n");
                }
            }
            // printf(ret_ASLR);
        }
        /* 启用DEP */
        else if(strcmp(test_items, "DEP")==0){
            char ret_DEP[CMD_RESULT_BUF_SIZE]={0};

            // ExecuteCMD("adb root", debug_log);
            // ExecuteCMD("adb shell", debug_log);  //adb shell进入后pclose()又退出了该进程所以后续命令依旧是在windows下执行，所以有问题
            // printf(debug_log);
            // ExecuteCMD("cat /proc/self/maps | grep -E \"heap|stack\"", ret_DEP);    //查询设备是否启用DEP
            // ExecuteCMD("exit", debug_log);

            // FILE *ptr = NULL;
            // ptr = popen("adb shell | cat /proc/self/maps | grep -E \"heap|stack\"", "r");
            // pclose(ptr);

            // ExecuteCMD("adb shell | cat /proc/self/maps | grep -E \"heap|stack\"", ret_DEP);    //查询设备是否启用DEP            
            if (sel != NULL){
                sprintf(cmd, "adb -s %s shell cat /proc/self/maps | find \"heap|stack\"", device);
                ExecuteCMD(cmd, ret_DEP);
            }else{                
                ExecuteCMD("adb shell cat /proc/self/maps | find \"heap|stack\"", ret_DEP);    //查询设备是否启用DEP
            }              
                       
            char* pch = NULL;
            // pch = strstr(ret_DEP, "x");
            pch = strchr(ret_DEP, 'x'); //TODO：有缺陷，后续可以考虑使用正则表达式去判断
            if (pch != NULL){
                // strcat(result, "success, 未启用DEP\n");
                strcat(result, "success,0\n");
            }else{                
                // strcat(result, "success, 已启用DEP\n");
                strcat(result, "success,1\n");
            }
        }
        /* 启用RELRO */
        else if(strcmp(test_items, "RELRO")==0){
            char ret_RELRO[CMD_RESULT_BUF_SIZE]={0};

            // if (sel != NULL){
            //     sprintf(cmd, "adb -s %s:5555 shell cat /proc/self/maps | find \"heap|stack\"", device);
            //     ExecuteCMD(cmd, ret_DEP);
            // }else{                
            //     ExecuteCMD("adb shell cat /proc/self/maps | find \"heap|stack\"", ret_DEP);    //查询设备是否启用DEP
            // }  
            ExecuteCMD("readelf -d libc.so | findstr BIND_NOW", ret_RELRO); // readelf -d libc.so | findstr NOW，使用者需要先有这个readelf工具
            // ExecuteCMD("adb shell cat /proc/sys/kernel/randomize_va_space", ret_RELRO);
            
            char* pch = NULL;
            pch = strstr(ret_RELRO, "BIND_NOW");
            if (pch != NULL){
                // strcat(result, "success, 已启用RELRO\n");
                strcat(result, "success,1\n");
            }else{
                // strcat(result, "success, 未启用RELRO\n");
                // printf("%s\n", "success,0");
                strcat(result, "success,0\n");
            }
        }
        /* 应用隔离 */
        else if(strcmp(test_items, "Application_isolation")==0){
            char ret_android_version[CMD_RESULT_BUF_SIZE]={0};
            char android_version[] = "4.0.0";   //char*改为char[]后Segmentation fault消失

            if (sel != NULL){
                sprintf(cmd, "adb -s %s shell getprop ro.build.version.release", device);
                ExecuteCMD(cmd, ret_android_version);
            }else{                
                ExecuteCMD("adb shell getprop ro.build.version.release", ret_android_version);  //应用隔离：安卓系统从版本4.1开始具备了应用隔离的机制
            }  
            
            int ret = compareVersions(ret_android_version, android_version);
            if (ret > 0) {
                // printf("设备具备应用隔离机制！\n");
                // strcat(result, "设备具备应用隔离机制！\n");
                strcat(result, "success,1\n");
            }else{
                // printf("设备没有应用隔离机制。\n");
                // strcat(result, "设备没有应用隔离机制。\n");
                strcat(result, "success,0\n");
            }
        }

        /* 应用安装验签 */
/*         else if(strcmp(test_items, "APK")==0){
            char ret_APK[CMD_RESULT_BUF_SIZE]={0};

            ExecuteCMD("adb install -r APK/app-release.apk", ret_APK); // 无法获取安装是否成功的结果信息，直接自动重定向到了标准输出中
            
            char* pch = NULL;
            pch = strstr(ret_APK, "INSTALL_FAILED_UPDATE_INCOMPATIBLE");
            if (pch != NULL){
                strcat(result, "success,1\n");
            }else{
                // printf("%s\n", "success,0");
                strcat(result, "success,0\n");
            }
        } */

        /* 截屏录屏保护 */
/*         else if(strcmp(test_items, "Scrcpy")==0){
            char ret_Scrcpy[CMD_RESULT_BUF_SIZE]={0};

            ExecuteCMD("C:\\Users\\admin\\Desktop\\scrcpy-win64-v2.1.1\\scrcpy.exe", ret_Scrcpy);  //
            if (ret_Scrcpy != NULL) {
                // printf("设备具备应用隔离机制！\n");
                // strcat(result, "设备具备应用隔离机制！\n");
                strcat(result, "success,1\n");
            }else {
                // printf("设备没有应用隔离机制。\n");
                // strcat(result, "设备没有应用隔离机制。\n");
                strcat(result, "success,0\n");
            }
        } */
        
    // }else{
    // }
    
    return;
}


/*
 * cmd：待执行命令
 * result：命令输出结果
 * 函数返回：0 成功；-1 失败；
 */
int ExecuteCMD(const char* cmd, char* result_)
{
    char result[CMD_RESULT_BUF_SIZE] = {0};
    char buf_temp[CMD_RESULT_BUF_SIZE] = {0};
    FILE *ptr = NULL;
    int iRet = -1;
    // printf("%d\n", __LINE__); 
    //popen: 开启子进程，建立管道，并运行指令，'r':从子进程获取结果，'w':向子进程写数据
    if((ptr = popen(cmd, "r")) != NULL)  //popen
    {
        // printf("%d\n", __LINE__); 
        // sleep(5);
            while(fgets(buf_temp, sizeof(buf_temp), ptr) != NULL)   //读取之后再次读取时判断是否没有读取到任何字符，是则返回空指针
            {
                if(strlen(result) + strlen(buf_temp) > CMD_RESULT_BUF_SIZE){
                    printf("shell返回结果超出buffer size,进行截断\n");
                    break;
                }
                strcat(result, buf_temp);  //字符串拼接 //char *strcat(char *dest, const char *src) 把 src 所指向的字符串追加到 dest 所指向的字符串的结尾。
            }
        strcpy(result_, result);
        pclose(ptr);
        ptr = NULL;
        iRet = 0;  // 处理成功
    }
    else
    {
        printf("popen %s error\n", cmd);
        iRet = -1; // 处理失败
    }
// printf("%d\n", __LINE__); 
    return iRet;
}

/* 比较版本号新旧 */
int compareVersions(char* version1, char* version2) 
{
    // 使用 strtok 函数按照 '.' 切分版本号
    char* token1 = strtok((char*)version1, ".");
    char* token2 = strtok((char*)version2, ".");

    while (token1 != NULL || token2 != NULL) {
        // 若版本号位数不同，则按照较短的那个版本号补充 '0'
        int num1 = token1 != NULL ? atoi(token1) : 0;
        int num2 = token2 != NULL ? atoi(token2) : 0;

        if (num1 > num2) {
            return 1;  // 版本号 version1 较新
        } else if (num1 < num2) {
            return -1;  // 版本号 version2 较新
        }

        // 继续比较下一位
        token1 = strtok(NULL, ".");
        token2 = strtok(NULL, ".");
    }

    return 0;  // 两个版本号相同
}


#endif




// #if 1
#ifdef __linux__    // Linux系统下的代码

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// #include <windows.h>
#include <time.h>

#define CMD_RESULT_BUF_SIZE 1024

char result[CMD_RESULT_BUF_SIZE]={0};
char debug_log[CMD_RESULT_BUF_SIZE]={0};

/*
 * cmd：待执行命令
 * result：命令输出结果
 * 函数返回：0 成功；-1 失败；
 */
int ExecuteCMD(const char* cmd, char* result_)
{
    char result[CMD_RESULT_BUF_SIZE] = {0};
    char buf_temp[CMD_RESULT_BUF_SIZE] = {0};
    FILE *ptr = NULL;
    int iRet = -1;

    //popen: 开启子进程，建立管道，并运行指令，'r':从子进程获取结果，'w':向子进程写数据
    if((ptr = popen(cmd, "r")) != NULL)  //popen
    {
            while(fgets(buf_temp, sizeof(buf_temp), ptr) != NULL)   //读取之后再次读取时判断是否没有读取到任何字符，是则返回空指针
            {
                if(strlen(result) + strlen(buf_temp) > CMD_RESULT_BUF_SIZE){
                    printf("shell返回结果超出buffer size,进行截断\n");
                    break;
                }
                strcat(result, buf_temp);  //字符串拼接 //char *strcat(char *dest, const char *src) 把 src 所指向的字符串追加到 dest 所指向的字符串的结尾。
            }
        strcpy(result_, result);
        pclose(ptr);
        ptr = NULL;
        iRet = 0;  // 处理成功
    }
    else
    {
        printf("popen %s error\n", cmd);
        iRet = -1; // 处理失败
    }

    return iRet;
}

/* 比较版本号新旧 */
int compareVersions(char* version1, char* version2) {
    // 使用 strtok 函数按照 '.' 切分版本号
    char* token1 = strtok((char*)version1, ".");
    char* token2 = strtok((char*)version2, ".");

    while (token1 != NULL || token2 != NULL) {
        // 若版本号位数不同，则按照较短的那个版本号补充 '0'
        int num1 = token1 != NULL ? atoi(token1) : 0;
        int num2 = token2 != NULL ? atoi(token2) : 0;

        if (num1 > num2) {
            return 1;  // 版本号 version1 较新
        } else if (num1 < num2) {
            return -1;  // 版本号 version2 较新
        }

        // 继续比较下一位
        token1 = strtok(NULL, ".");
        token2 = strtok(NULL, ".");
    }

    return 0;  // 两个版本号相同
}

/* PCI合规检测 */
void Compliance(char * test_items, const char * device)
{
    /* 设备识别 */
    if(strcmp(device, "P201221720021") == 0){
        /* PCI合规检测项 */
        /* 24小时重启 */
        if(strcmp(test_items, "24hour")==0){
            char ret_boot_time[CMD_RESULT_BUF_SIZE]={0};

            ExecuteCMD("adb shell cat /proc/stat | grep \"btime\"", ret_boot_time); //获取设备开机时间-验证24小时重启
            {
                // 获取当前时间戳
                time_t current_time = time(NULL);   // printf("%d\n", current_time);
                // 获取设备开机时间戳
                time_t num1 = atoll((const char *)ret_boot_time);
                time_t boot_time = num1; // 这里替换成实际获取到的设备开机时间戳
                
                // 计算时间差值（以秒为单位）
                double time_diff = difftime(current_time, boot_time);

                // 判断时间差是否接近24小时（容许误差范围为60秒）
                if (time_diff >= 86400 - 60 && time_diff <= 86400 + 60) {
                    // strcat(result, "设备具备24小时自动重启功能!\n");  //char *strcat(char *dest, const char *src) 把 src 所指向的字符串追加到 dest 所指向的字符串的结尾。
                    strcat(result, "success,1\n"); 
                } else {
                    // strcat(result, "设备没有24小时自动重启功能。\n");
                    // printf("设备没有24小时自动重启功能。\n");
                    strcat(result, "success,0\n");
                }
            }
        }
        /* 启用ASLR */
        if(strcmp(test_items, "ASLR")==0){
            char ret_ASLR[CMD_RESULT_BUF_SIZE]={0};

            // popen("adb root", "r");
            // popen("adb shell", "r");
            ExecuteCMD("adb shell cat /proc/sys/kernel/randomize_va_space", ret_ASLR);    //查询设备是否启用ASLR
            
            char* pch = NULL;
            // pch = strstr(ret_ASLR, "2");
            pch = strchr(ret_ASLR, '2');
            if (pch != NULL)
                // strcat(result, "success, 已启用ASLR\n");
                strcat(result, "success,1\n");
            else{
                // strcat(result, ret_ASLR);
                // strcat(result, "success, 未启用ASLR\n");
                strcat(result, "success,0\n");
            }
        }
        /* 启用DEP */
        if(strcmp(test_items, "DEP")==0){
            char ret_DEP[CMD_RESULT_BUF_SIZE]={0};

            // ExecuteCMD("adb root", debug_log);
            // ExecuteCMD("adb shell", debug_log);  //adb shell进入后pclose()又退出了该进程所以后续命令依旧是在windows下执行，所以有问题
            // printf(debug_log);
            // ExecuteCMD("cat /proc/self/maps | grep -E \"heap|stack\"", ret_DEP);    //查询设备是否启用DEP
            // ExecuteCMD("exit", debug_log);

            // FILE *ptr = NULL;
            // ptr = popen("adb shell | cat /proc/self/maps | grep -E \"heap|stack\"", "r");
            // pclose(ptr);

            // ExecuteCMD("adb shell | cat /proc/self/maps | grep -E \"heap|stack\"", ret_DEP);    //查询设备是否启用DEP            
            ExecuteCMD("adb shell cat /proc/self/maps | grep \"heap|stack\"", ret_DEP);    //查询设备是否启用DEP            
            
            char* pch = NULL;
            // pch = strstr(ret_DEP, "x");
            pch = strchr(ret_DEP, 'x'); //TODO：有缺陷，后续可以考虑使用正则表达式去判断
            if (pch != NULL)
                // strcat(result, "success, 未启用DEP\n");
                strcat(result, "success,0\n");
            else{                
                // strcat(result, ret_DEP);
                // strcat(result, "success, 已启用DEP\n");
                strcat(result, "success,1\n");
                // printf("%s\n", "success,0");
                // printf(ret_DEP);
                // printf("%d\n", __LINE__);
            }
        }
        /* 启用RELRO */
        if(strcmp(test_items, "RELRO")==0){
            char ret_RELRO[CMD_RESULT_BUF_SIZE]={0};

            ExecuteCMD("readelf -d libc.so | grep BIND_NOW", ret_RELRO); // readelf -d libc.so | findstr NOW，使用者需要先有这个readelf工具
            // ExecuteCMD("adb shell cat /proc/sys/kernel/randomize_va_space", ret_RELRO);
            
            char* pch = NULL;
            pch = strstr(ret_RELRO, "BIND_NOW");
            if (pch != NULL)
                // strcat(result, "success, 已启用RELRO\n");
                strcat(result, "success,1\n");
            else{
                // strcat(result, "success, 未启用RELRO\n");
                // printf("%s\n", "success,0");
                strcat(result, "success,0\n");
            }
        }
        /* 应用隔离 */
        if(strcmp(test_items, "Application_isolation")==0){
            char ret_android_version[CMD_RESULT_BUF_SIZE]={0};
            char android_version[] = "4.0.0";   //char*改为char[]后Segmentation fault消失

            ExecuteCMD("adb shell getprop ro.build.version.release", ret_android_version);  //应用隔离：安卓系统从版本4.1开始具备了应用隔离的机制
            int ret = compareVersions(ret_android_version, android_version);
            if (ret > 0) {
                // printf("设备具备应用隔离机制！\n");
                // strcat(result, "设备具备应用隔离机制！\n");
                strcat(result, "success,1\n");
            } 
            else {
                // printf("设备没有应用隔离机制。\n");
                // strcat(result, "设备没有应用隔离机制。\n");
                strcat(result, "success,0\n");
            }
        }
        


    }
    
    return;
}


int main(int argc, char *argv[])
{
	// printf("argc = %d\n", argc);

    // for(int i=0; i<argc; ++i)
    // 	printf("%s\n", argv[i]);	
        
    // char result1[CMD_RESULT_BUF_SIZE]={0};
    // if("设备已连接再检测")

    if (argc < 2) {
        printf("Usage: %s <command>\n", argv[0]);
        return 1;
    }

    char* device = NULL;
    /* compliance.exe --test 24hour|version_compare --device VC02D36E20003 */
    if(argv[4] != NULL){
        device = argv[4];
    }
    // else{
    //     ExecuteCMD("adb devices", result);

    //     printf("Usage: %s <command>\n");

    //     memset(result, 0, sizeof(result));
    // }

    /* 使用 strtok 函数按照 '|' 切分命令参数 */
    char* cmd = strtok(argv[2], "|");
    // char* cmd1 = cmd;
    while (cmd != NULL) {
        // printf("%s\n", cmd);
        Compliance(cmd, device);

        // for(int i=0; i<argc; ++i){
        //     Compliance(cmd, device);
        // }        

        // 继续比较下一位
        cmd = strtok(NULL, "|");
    }

    // printf("%d\n", __LINE__); 
    // printf("%s\n%s\n%s\n", result1, result2, result3);
    // printf("%s\n%s\n", result, cmd1);
    printf("%s\n", result);

    system("pause");    // 防止运行后自动退出，需头文件stdlib.h

    return 0;
}
    

#endif



