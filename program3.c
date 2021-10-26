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

int sigint_event_handler(const char* pipe_name, const char* rm_fifo);

int main(int argc, char* argv[])
{
    int fd1, num, prev_num = 0, is_first_app_closed = 0;		// fd1 - дескриптор именнованного канала, num - число, которое будет в этом канале,
									// prev_num - предыдущее число, которое было в этом канале (нужно для системы прерываний)
    char buff[50], buff2[5]; 						// массив, в котором будут храниться символы из именнованного канала fifo
    while(1)
    {
	if((fd1 = open("fifo", O_RDONLY | O_NONBLOCK)) == -1) 		// проверка на открытие именнованного канала fifo (программа 1)
	{
	    fprintf(stderr, "Can't open fifo!\n"); 			// вывод об ошибке
	    break;
	    return NOT_OPENED_FIFO1; 					// возвращение кода ошибки
	}
	read(fd1, buff, 50); 						// запись данных с канала fifo в buff
        num = atoi(buff); 						// перевод данных из char* в int
        if(num != prev_num) 						// если, число внутри канала fifo изменилось, то выводим его в stdout, иначе ничего не делаем
	{
            fprintf(stdout, "%d\n", num);
	    prev_num = num; 						// переприсваиваем prev_num
	}
	close(fd1); 							// закрываем канал
	is_first_app_closed = sigint_event_handler("fappexit", "fifo"); // вызываем функцию проверки закрытия 1ой программы
	if(is_first_app_closed)						// если пришла 1
	{
	    fprintf(stdout, "First program was done...\n");		// выводим соответствующее сообщение
	    break;							// выходим из бесконечного цикла
	}
    }
    return 0;
}


int sigint_event_handler(const char* pipe_name, const char* rm_fifo) 	// функция, обрабатывающая завершение 1ой или 2ой программы (в зависимости от имени канала в pipe_name)
{
    int fd;								// дескриптор канала
    char buff[5];							// хранилище данных в канале

    if((fd = open(pipe_name, O_RDONLY | O_NONBLOCK)) != -1)		// если канал открылся
    {
        read(fd, buff, 5);						// считываем данные из канала в buff
        if(atoi(buff) == -1)						// если данные == -1
        {
            close(fd);							// закрываем канал
            unlink(pipe_name);						// удаляем канал(по имени)
            unlink(rm_fifo);						// удаляем канал для чтения времени
	    return 1;							// возвращаем 1, то есть приложение закрылось
        }
    }
    return 0;								// если канал не открылся или там считалась не -1, возвращаем 0
}
