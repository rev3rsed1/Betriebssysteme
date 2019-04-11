#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>

char a, b, c;


void function() {
    char f_a, f_b, f_c;
    char *f_d, *f_e, *f_f;

    f_d = malloc(sizeof(char));
    f_e = malloc(sizeof(char));
    f_f = malloc(sizeof(char));

    printf("%10s: a=%p, b=%p, c=%p \n", "f_stack", &f_a, &f_b, &f_c);
    printf("%10s: d=%p, e=%p, f=%p \n", "f_heap", f_d, f_e, f_f);

    free(f_d);
    free(f_e);
    free(f_f);
}

int main() {

    printf("%10s: %p \n","main", &main);

    printf("%10s: %p \n\n","function", &function);

    printf("%10s: a=%p, b=%p, c=%p \n", "globals:", &a, &b, &c);

    char m_a, m_b, m_c;
    char *m_d, *m_e, *m_f;

    m_d = malloc(sizeof(char));
    m_e = malloc(sizeof(char));
    m_f = malloc(sizeof(char));

    printf("%10s: a=%p, b=%p, c=%p \n", "m_stack", &m_a, &m_b, &m_c);
    printf("%10s: d=%p, e=%p, f=%p \n", "m_heap", m_d, m_e, m_f);

    free(m_d);
    free(m_e);
    free(m_f);

    function();

    struct rlimit limit;
    getrlimit(RLIMIT_STACK, &limit);
    
    printf("\n%12s: soft=%lu, hard=%lu \n", "stack-limit",
           limit.rlim_cur, limit.rlim_max);

    getrlimit(RLIMIT_DATA, &limit);
    printf("%12s: soft=%lu, hard=%lu \n", "heap-limit",
           limit.rlim_cur, limit.rlim_max);

    return 0;
}
