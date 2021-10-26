#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define NOT_OPENED_FIFO1 1

int main(int argc, char* argv[])
{
    int fd1, num, prev_num = 0; // fd1 - дескриптор именнованного канала, num - число, которое будет в этом канале, prev_num - предыдущее число, которое было в этом канале (нужно для системы прерываний)
    char buff[50]; // массив, в котором будут храниться символы из именнованного канала fifo
    while(1)
    {
	if((fd1 = open("fifo", O_RDONLY | O_NONBLOCK)) == -1) // проверка на открытие именнованного канала fifo (программа 1)
	{
	    fprintf(stderr, "Can't open fifo!\n"); // вывод об ошибке
	    break;
	    return NOT_OPENED_FIFO1; // возвращение кода ошибки
	}
	read(fd1, buff, 50); // запись данных с канала fifo в buff
        num = atoi(buff); // перевод данных из char* в int
	if(num == -1)
	{
	    close(fd1);
	    unlink("fifo");
	    fprintf(stdout, "First program was done...\n");
	    return 0;
	}
        if(num != prev_num) // если, число внутри канала fifo изменилось, то выводим его в stdout, иначе ничего не делаем
	{
            fprintf(stdout, "%d\n", num);
	    prev_num = num; // переприсваиваем prev_num
	}
	close(fd1); // закрываем канал
    }
    return 0;
}
