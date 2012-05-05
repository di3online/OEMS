/**
 * @file test_time.cpp
 * @brief 
 * @author LaiSHZH
 * @version 1.0
 * @date 2012-05-03
 */
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cstring>


/**
 * @brief 
 *      convert the time string to time_t
 *
 * @param str
 *      time string whose format is like 
 *      2012-05-03 22:33:13
 */
time_t
convert_time(const char *str)
{
    struct tm time;

    sscanf(str, "%d-%d-%d %d:%d:%d", 
            &time.tm_year, 
            &time.tm_mon,
            &time.tm_mday,
            &time.tm_hour,
            &time.tm_min,
            &time.tm_sec);
    time.tm_year -= 1900;
    time.tm_mon -= 1;

    return mktime(&time);

}

void 
convert_time(char *to, size_t size, const char *from)
{
    time_t timeval = strtol(from, NULL, 10);
    struct tm time;

    localtime_r(&timeval, &time);

    strftime(to, size, "%F %T", &time);

    return ;
}

int main()
{
    time_t second;

    char buf[] = "2012-05-03 22:33:50";

    second = convert_time(buf);

    printf("second = %lu\n", second);

    printf("second = %lu\n", time(NULL));

    char buf1[30];
    snprintf(buf, sizeof(buf), "%ld", second);

    convert_time(buf1, sizeof(buf1), buf);

    printf("time: %s\n", buf1);

    snprintf(buf, sizeof(buf), "%ld", time(NULL));
    convert_time(buf1, sizeof(buf1), buf);
    
    printf("time: %s\n", buf1);

    return 0;
}
