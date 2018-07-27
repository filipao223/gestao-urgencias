/*Autores:
  João Filipe Sequeira Montenegro Nº 2016228672
  João Miguel Rainho Mendes Nº 2016230975
*/
#ifndef LOG_H
#define LOG_H

void write_to_log(char*);
//Some log messages are written before semaphores are initialized
void write_to_log_no_sems(char*);
void return_time_format(char*);

#endif //LOG_H
