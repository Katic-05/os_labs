#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <signal.h>

#define NO_INPUT_ARGS 1
#define FILE_DOESNT_EXIST 2

void sigint_event(int fifo);

int main(int argc, char* argv[])
{
    char buff[2]; // переменная для считывания данных из gpio17
    char time_buff[50]; // массив символов, в котором будет храниться время, в которое произошёл хлопок
    bool prev = 0, curr = 0; // булевые переменные для системы прерываний записи в файл/выводе в именованный канал
    time_t ttime; // переменная, в которую будет записываться время хлопка
    int fd_fifo; // дескриптор именованного канала
    FILE* fp = NULL; // файл, название которого указано в argv[1]
    FILE* in = NULL; // файл со значением в gpio17

    struct sigaction sa;
    sa.sa_handler = &sigint_event;
    sa.sa_flags = SA_RESTART;

    sigaction(SIGINT, &sa, NULL);

    if(argc != 2) // обрабатывание ошибки об отсутствии входного аргумента
    {
	fprintf(stderr, "No input arguments!\n");
        return NO_INPUT_ARGS;
    }
    unlink("fifo"); // удаляем канал fifo, если был
    if(mkfifo("fifo", 0777) == -1) // создание именованного канала и проверка создался ли, вывод о соответствующей ошибке
	fprintf(stderr, "Can't create fifo!\n");
    while(1)
    {
        //in = fopen("/sys/class/gpio/gpio17/value", "r"); // то же самое, что и ниже, только для raspberry pi
	in = fopen("/home/oxygen1u8/gpio17/value", "r"); // открываем на чтение файл со значением в gpio17
        fgets(buff, 2, in); // считывание из файла
        if(atoi(buff) == 1) // если в файле была 1
        {
	    curr = 1; // curr - текущее состояние, присваиваем 1
            ttime = time(NULL); // замеряем время, в которое прозвучал хлопок
	    if(curr != prev) // если предыдущее состояние gpio17 не равно нынешнему(если предыдущее 0), выводим в файл и именованный канал
	    {
		fd_fifo = open("fifo", O_RDWR); // открываем канал на чтение и запись
		if(fd_fifo == -1) // если не открылся, выводим ошибку
		    fprintf(stderr, "Can't open fifo!\n");
		sprintf(time_buff, "%ld", ttime); // записываем время хлопка в time_buff
		if(write(fd_fifo, time_buff, strlen(time_buff)) == -1) // запись в именованный канал и тут же проверка, записалось ли
		    fprintf(stderr, "Not writing to fifo!\n"); // уведомление об ошибке
		close(fd_fifo); // закрываем канал
            	fp = fopen(argv[1], "a"); // открываем файл
		if(fp == NULL) // если не открылся
		{
		    fprintf(stderr, "File doesn't exist!\n");
		    return FILE_DOESNT_EXIST; // вывод кода ошибки и завершение программы
		}
            	fprintf(fp, "%s\n", time_buff); // запись в файл
            	fclose(fp); // закрываем файл
            	fp = NULL; // указатель на файл присваиваем к NULL
	    }
	    prev = 1; // prev - предыдущее состояние gpio17, присваиваем 1
        }
	else
	    curr = prev = 0; // если в gpio17, нынешнее и предыдущее - нули
        fclose(in); // закрываем файл gpio17
        in = NULL; // присваиваем NULL
    }
    return 0;
}

void sigint_event(int fifo)
{
    int fd_fifo = open("fifo", O_RDWR);
    char buff[3] = "-1\0", buff2[10];
    if(fd_fifo == -1)
    {
	fprintf(stderr, "Can't open fifo\n");
	return;
    }
    read(fd_fifo, buff2, 10);
    write(fd_fifo, buff, strlen(buff));
    close(fd_fifo);
}
