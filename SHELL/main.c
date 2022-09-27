#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

struct new_proc{
    char* fin;
    char* fout;
    int fout_arg;
    char mode;
    int argc;
    char* argv[10];
};

int conveyer(struct new_proc* massive[], int j, int n, int k, int fone_arg);
int input(struct new_proc* str);
int input_string(char** s, char f);
void free_all(struct new_proc* massive[], int n);
void output_mas(struct new_proc* massive[], int n);
int new_SHELL(char* s);
int read_until(char* argv[]);

volatile int flag = 0;

int main(int argc, char*argv[])
{
    int flag = 0;
    if(argc > 1)
        flag = 1;
    if(!flag)
        printf("Hello, I am a poor substitute of SHELL!\n");
    int k, n, err, num, j;
    struct new_proc* str;
    struct new_proc* massive[20];
    while(1){
        if(!flag)
            printf("\nInput some command: ");
        k = 0;
        while(k == 0){
            //(struct new_proc**) massive = (struct new_proc**) realloc(massive, sizeof(struct new_proc*));
            n = 0;
            while(k == 0){
                str = (struct new_proc*) malloc(sizeof(struct new_proc));
                str->fin = NULL;
                str->fout = NULL;
                str->fout_arg = 0;
                str->argc = 0;
                str->mode = '\0';
                *(str->argv) = NULL;
                k = input(str);
                massive[n] = str;
                if(*(str->argv) == NULL)
                    free(massive[n]);
                else
                    n++;
            }
            if(k != 2){
                err = 0;
                for(int i = 0; i < n; i++){
                    num = 1;
                    j = i;
                    while(massive[i]->mode == '|'){
                        i++;
                        num++;
                    }
                    if(i >= n){
                        printf("Incorrect command1\n");
                        k = 2;
                        break;
                    }
                    if(massive[i]->fout_arg == -1){
                        err = new_SHELL(*(massive[i]->argv));
                    }
                    else if(k >= 4 && (massive[i]->mode == '1' || massive[i]->mode == '2')){
                        err = conveyer(massive, j, num, k, 1);
                    }
                    else
                        err = conveyer(massive, j, num, k, 0);
                    if(massive[i]->mode == '1' && err != 0) break;
                    else if(massive[i]->mode == '2' && err == 0) break;
                    else if(massive[i]->mode != ';' && massive[i]->mode != '&' && massive[i]->mode != '\0' && massive[i]->mode != '1' && massive[i]->mode != '2' && massive[i]->mode != '3' && massive[i]->mode != '4'){
                        printf("Incorrect command2 %c\n", massive[i]->mode);
                        k = 2;
                        break;
                    }
                }
            }
            if(flag != 0 && (k == 1 || k == 5)){
                exit(err);
            }
            free_all(massive, n);
            if(k == 2){
                char c;
                while((c = getchar()) != '\n');
                if(flag != 0)
                    exit(0);
            }
            else if(k == 3 || k == 4)
                k = 0;
        }
    }
    return 0;
}


int conveyer(struct new_proc* massive[], int j, int n, int k, int fone_arg)
{
    if(k == 5) k = 4;
    int fd_in = 0, fd_out = 1, f_null;
    if(massive[j]->fin != NULL){
        fd_in = open(massive[j]->fin, O_RDONLY);
        if(fd_in <= 0){
            printf("Cannot open file %s\n", massive[j]->fin);
            return 1;
        }
    }
    if(massive[j]->fout != NULL){
        if(massive[j]->fout_arg == 0){
            fd_out = open(massive[j]->fout, O_RDWR | O_TRUNC | O_CREAT, 0666);
        }
        else{
            fd_out = open(massive[j]->fout, O_RDWR | O_CREAT, 0666);
            lseek(fd_out, 0, SEEK_END);
        }
        if(fd_out <= 0){
            printf("Cannot open file %s\n", massive[j]->fout);
            return 1;
        }
    }
    if(massive[j + n - 1]->fin != NULL && fd_in == 0){
        fd_in = open(massive[j]->fin, O_RDONLY);
        if(fd_in <= 0){
            printf("Cannot open file %s\n", massive[j + n - 1]->fin);
            return 1;
        }
    }
    if(massive[j + n - 1]->fout != NULL && fd_out == 1){
        if(massive[j + n - 1]->fout_arg == 0){
            fd_out = open(massive[j + n - 1]->fout, O_RDWR | O_TRUNC | O_CREAT, 0666);
        }
        else{
            fd_out = open(massive[j + n - 1]->fout, O_RDWR | O_CREAT, 0666);
            lseek(fd_out, 0, SEEK_END);
        }
        if(fd_out <= 0){
            printf("Cannot open file %s\n", massive[j + n - 1]->fout);
            return 1;
        }
    }
    //printf("FD_IN = %d  FD_OUT = %d, fout = %s\n", fd_in, fd_out, massive[j]->fout);
    int fd[2], status = 0;
    pid_t p, q = 0;
    if(fd_in == 0){
        fd_in = dup(0);
    }
    for(int i = j; i < j + n - 1; i++){
        pipe(fd);
        if(k == 4){
            if((q = fork()) < 0){
                printf("Error in fork\n");
                return 1;
            }
            if(q == 0){
                setpgid(0, 0);
                f_null = open("/dev/null", O_RDONLY);
                dup2(f_null, 0);
                close(f_null);
            }
        }
        if((k == 4 && q == 0) || k != 4){
            if((p = fork()) < 0){
                printf("Error in fork\n");
                return 1;
            }
            if(k == 4 && q == 0 && p != 0)
                exit(0);
            if(p == 0){
                if(k == 4){

                }
                close(fd[0]);    // закрываю канал на чтение
                dup2(fd_in, 0);  // перенаправление ввода на ввод из fd_in
                close(fd_in);    // закрываю fd_in
                dup2(fd[1], 1);  // перенаправление вывода на вывод в канал
                close(fd[1]);    // закрытие канала на запись
                execvp(*(massive[i]->argv), massive[i]->argv);
                printf("Error in exec\n");
                exit(1);
            }
        }
        dup2(fd[0], fd_in);  // в fd_in теперь лежит чтение из канала
        close(fd[0]);        // закрываю чтение для отца
        close(fd[1]);        // закрываю записть для отца
    }
    if(k == 4){
        if((q = fork()) < 0){
            printf("Error in fork\n");
            return 1;
        }
        if(q == 0){        // сын для фонового
            setpgid(0, 0);
            f_null = open("/dev/null", O_RDONLY);
            dup2(f_null, 0);
            close(f_null);
        }
    }
    if((k == 4 && q == 0) || k != 4){
        if((p = fork()) < 0){          // породили внука для фонового, или сына для обычного
            printf("Error in fork\n");
            return 1;
        }
        if(k == 4 && q == 0 && p != 0){ // завершаем сына для фонового процесса
            if(fone_arg != 0){
                wait(&status);
                if(status != 0)
                    exit(1);
                else
                    exit(0);
            }
            exit(0);
        }
        if(p == 0){                    // внук для фонового или сын для обычного
            dup2(fd_in, 0);
            close(fd_in);
            if(fd_out != 1){
                dup2(fd_out, 1);
                close(fd_out);
            }
            execvp(*(massive[j + n - 1]->argv), massive[j + n - 1]->argv);
            printf("Error in exec\n");
            exit(1);
        }
    } // отец
    close(fd_in);
    int f = 0;
    if(fd_out != 1)
        close(fd_out);
    while(wait(&status) >= 0){
        if(status != 0)
            f = 1;
    }
    return f;
}


int new_SHELL(char* s)
{
    char c;
    pid_t p;
    int fd[2];
    pipe(fd);
    if((p = fork()) < 0){
        printf("Error in fork\n");
        return 0;
    }
    if(p == 0){
        close(fd[1]);
        dup2(fd[0], 0);
        close(fd[0]);
        execlp("./a.out", "./a.out", "argument", NULL);
        printf("Error in exec\n");
        return -1;
    }
    else{
        close(fd[0]);
        int i = 0;
        while(*(s + i) != '\n'){
            c = *(s + i);
            write(fd[1], &c, 1);
            i++;
        }
        c = '\n';
        write(fd[1], &c, 1);
        close(fd[1]);
        int status;
        waitpid(p, &status, 0);
        if(status != 0)
            return 1;
        else
            return 0;
    }
}


int read_until(char** s)
{
    char c;
    int i = 0, len = 4, s1 = 0, s2 = 0;
    *s = (char*) malloc(4);
    while(s1 >= s2 && (c = getchar()) != '\n'){
        *(*s + i) = c;
        if(c == '(')
            s1++;
        if(c == ')')
            s2++;
        if(i % 4 == 3){
            *s = realloc(*s, len + 4);
            len += 4;
        }
        i++;
    }
    if(c == '\n')
        return 2;
    *(*s + i - 1) = '\n';
    printf("%s", *s);
    return 0;
}


void output_mas(struct new_proc* massive[], int n)
{
    if(n == 0)
        return;
    printf("\n----------%d--------\n", n);
    for(int i = 0; i < n; i++){
            printf("%s\n%s\n%d\n%c\n%d\n%s\n%s\n%s\n",
                (massive[i])->fin, (massive[i])->fout, (massive[i])->fout_arg, (massive[i])->mode, (massive[i])->argc,
                 *(massive[i]->argv), *(massive[i]->argv + 1), *(massive[i]->argv + 2));
        }
}


void free_all(struct new_proc* massive[], int n)
{
    for(int i = 0; i < n; i++){
        free(massive[i]->fin);
        free(massive[i]->fout);
        for(int j = 0; j < massive[i]->argc; j++){
            free(*(massive[i]->argv + j));
        }
        free(massive[i]);
    }
}

/*

------------------------------------------------------------------------------------
                                        ВВОД
*/


int input(struct new_proc* str)
{
    char c;
    int k = 0;
    while(k == 0 && (c = getchar()) != '\n'){
        if(c == ' '){
            continue;
        }
        else if(c == '('){
            str->fout_arg = -1;
            k = read_until(str->argv);
            str->argc++;
        }
        else if(c == ')' && flag == 0){
            k = 2;
            printf("Incorrect command3\n");
            break;
        }
        else if(c == '<'){
            k = input_string(&(str->fin), '\0');
            if(k == 2){
                printf("Incorrect command4\n");
                break;
            }
            if(k == 1)
                break;
        }
        else if(c == '>'){
            c = getchar();
            if(c == '>'){
                str->fout_arg = 1;
                k = input_string(&(str->fout), '\0');
            }
            else
                k = input_string(&(str->fout), c);
            if(k == 2){
                printf("Incorrect command5\n");
                break;
            }
            if(k  == 1)
                break;
        }
        else if(c == ';'){
            str->mode = ';';
            k = 3;
            break;
        }
        else if(c == '|'){
            c = getchar();
            if(c == ' '){
                str->mode = '|';
                k = 0;
                break;
            }
            else if(c == '|'){
                k = 0;
                str->mode = '2';
                break;
            }
            else{
                printf("Incorrect command, put space between operands1\n");
                k = 2;
                break;
            }
        }
        else if(c == '&'){
            c = getchar();
            if(c == ' ' || c == '\n'){
                k = 4;
                str->mode = '&';
                break;
            }
            else if(c == '&'){
                k = 0;
                str->mode = '1';
                break;
            }
            else{
                printf("Incorrect command, put space between operands2\n");
                k = 2;
                break;
            }
        }
        else{
           // (char**) str->argv = (char**) realloc((char**) str->argv, (str->argc + 1) * sizeof(char*));
            k = input_string(str->argv + str->argc, c);
            (str->argc)++;
            if(k != 0)
                break;
        }
    }
    *(str->argv + str->argc) = NULL;
    if(c == '\n' && k == 4)
        k = 5;
    else if(c == '\n')
        k = 1;
    return k;
}

int input_string(char** s, char f)
{
    char c = ' ';
    if(f != '\0')
        c = f;
    while(c == ' ')
        c = getchar();
    if(c == '\n'){
        printf("Incorrect command6\n");
        return 2;
    }
    *s = (char*) malloc(4);
    **s = c;
    int i = 1, len = 4;
    while((c = getchar()) != ' ' && c != '\n'){
        if(c == '&' || c == '|' || c == ';' || c == '<' || c == '>' || c == '(' || c == ')'){
            printf("Incorrect command, put space between operands3\n");
            return 2;
        }
        *(*s + i) = c;
        if(i % 4 == 3){
            *s = realloc(*s, len + 4);
            len += 4;
        }
        i++;
    }
    *(*s + i) = '\0';
    if(c == '\n')
        return 1;
    return 0;
}
