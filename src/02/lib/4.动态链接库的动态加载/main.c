#include <stdio.h>
#include <dlfcn.h> /*动态加载库的头文件*/

int main(void)
{
  char src[]="Hello Dymatic";
  /*函数指针变量*/
  int (*pStrLenFun)(char *str);

  void *phandle = NULL;
  char *perr = NULL;
  
  /*打开动态库*/
  phandle = dlopen("./libstr.so", RTLD_LAZY);
  if(!phandle)
  {
    printf("Failed Load library!\n");
  }
  
  /*检测错误*/
  perr = dlerror();
  if(perr != NULL)
  {
    printf("%s\n",perr);
    return 0;
  }
  
  /*获取动态库中的函数StrLen的地址，赋值给函数指针*/
  pStrLenFun = dlsym(phandle, "StrLen");
  perr = dlerror();
  if(perr != NULL)
  {
    printf("%s\n",perr);
    return 0;
  }

  /*通过函数指针调用库内函数*/
  printf("the string length is: %d\n",pStrLenFun(src));
  
  /*关闭动态库*/
  dlclose(phandle);

  return 0;
}
