#ifdef COMMENTS_ENABLED
#define COMMENT(msg)       printf("%s:%d " msg "\n", __FILE__, __LINE__)
#define COMMENT1(msg,x)    printf("%s:%d " msg "\n", __FILE__, __LINE__, x)
#define COMMENT2(msg,x,y)  printf("%s:%d " msg "\n", __FILE__, __LINE__, x, y)
#else
#define COMMENT(msg)
#define COMMENT1(msg,x)
#define COMMENT2(msg,x,y)
#endif
