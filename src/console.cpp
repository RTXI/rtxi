
#include <rtxi_config.h>

#include <debug.h>
#include <unistd.h>
#include <sys/wait.h>
#if HAVE_READLINE_READLINE_H
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include <console.h>
#include <main_window.h>

extern "C"
{
     static void *static_run (void *arg)
     {
	  ConsoleIO *io;

	  io = (ConsoleIO *)arg;

	  io->run();


	  return NULL;
     }

     static void *static_input_func (void *arg)
     {
	  ConsoleIO *io;

	  io = (ConsoleIO *)arg;
	  io->input_func();

	  return NULL;
     }

     static void *static_output_func (void *arg)
     {
	  ConsoleIO *io;

	  io = (ConsoleIO *)arg;
	  io->output_func();

	  return NULL;
     }
}

ConsoleIO::ConsoleIO (int id, char *prompt)
{
     this->id=id;
     this->prompt=prompt;
     this->requested = false;

     pipe(ifd);
     pipe(ofd);
}

ConsoleIO::~ConsoleIO (void)
{
     pthread_cancel (output_tid);
     pthread_cancel (input_tid);
     close(ofd[0]);
     close(ifd[1]);
     close(ofd[1]);
     close(ifd[0]);
}

#if HAVE_READLINE_READLINE_H
int console_readline (char **line);
#endif

void ConsoleIO::input_func (void)
{
     int s;
     size_t insize, i;
     const unsigned int max = 8;
     char inbuf[max];
     
     insize = 0;
     i = 0;

     insize = read (ifd[0],inbuf,max);
     while (insize > 0) 
     {
	  i = 0;
	  while (i < insize)
	  {
	       s = write(STDOUT_FILENO, inbuf+i, insize-i);
	       if (s < 0) return;
	       i += s;
	  }
	  insize = read(ifd[0],inbuf,max);
     }
}

#if HAVE_READLINE_READLINE_H
int console_readline (char **line, char *prompt)
{
     char *istr; 
     
     //rl_num_chars_to_read = 1;
     istr = readline(prompt);

     if ((istr != NULL))
     {
	  if (*istr != '\0') add_history (istr);

	  *line = istr;
	  return strlen(istr);
     }

     return -1;
}
#endif

void ConsoleIO::output_func (void)
{
     int s;
     const unsigned int max = 32;
     size_t outsize, i; 
     char outbuf[max+1];  
     char *line;

     line = NULL;
     memset (outbuf,0,max+1);

#if HAVE_READLINE_READLINE_H
     outsize = console_readline (&line, prompt);
#else
     outsize = read(STDIN_FILENO,(void *)outbuf,max);
     line = outbuf;
#endif
     while (outsize >= 0) 
     {
	  i = 0;
	  while (i < outsize)
	  {
	       s = write (ofd[1],line+i,outsize-i);
	       if (s < 0) return;
	       i += s;
	  }
	  if (*(line+i-1) != '\n') 
	  {
	       s = write (ofd[1],"\n",1);
	       if (s < 0) return;
	  }
#if HAVE_READLINE_READLINE_H
	  free (line);
	  outsize = console_readline (&line, prompt);
#else
	  outsize = read(STDIN_FILENO,(void *)outbuf,max);
	  line = outbuf;
#endif
     }
}

void ConsoleIO::run (void)
{
     int status;
#if HAVE_READLINE_READLINE_H
     rl_initialize();
#endif

     if ((status = pthread_create (&input_tid, NULL, static_input_func, this)) != 0)
	  return;
     if ((status = pthread_create (&output_tid, NULL, static_output_func, this)) != 0)
	  return;

     pthread_join(input_tid,NULL);
     pthread_join(output_tid,NULL);
}

int ConsoleIO::getInputFd (void)
{
     requested = true;
     return ifd[1];
}

int ConsoleIO::getOutputFd (void)
{
     requested = true;
     return ofd[0];
}

bool ConsoleIO::isRequested (void)
{
     return requested;
}

unsigned int ConsoleIO::getId (void)
{
     return id;
}

Console::Console(int nsessions, char *prompt): mutex(Mutex::RECURSIVE) 
{
     int i = 0;

     if (!(nsessions > 0)) return;

     this->nsessions = nsessions;

     cios.push_back(new ConsoleIO (0, prompt));

     if ((qon = qonsole_create ((MainWindow::getInstance())->centralWidget(),
				"rtxi-console", "RTXI Console",
				static_run, cios[0])) == NULL) return;
     i = 1;
     while (i < nsessions)
     {
	  cios.push_back(new ConsoleIO (i, prompt));
	  qon->newSession(static_run,(cios[i]));
	  i++;
     }

     menuID = MainWindow::getInstance()->createControlMenuItem("Console",this,SLOT(show(void)));    
}

Console::~Console(void) 
{
     int i;

     MainWindow::getInstance()->removeControlMenuItem(menuID);

     for (i=0; i<nsessions; i++)
     {
	  delete cios[i];
     }
     delete qon;

}

void Console::show (void)
{
     if (qon != NULL)
	  qonsole_show (qon);
}

static Mutex mutex;
Console *Console::instance = NULL;

Console *Console::getInstance(void) 
{
     if(instance != NULL)
	  return instance;
     
     Mutex::Locker lock(&::mutex);
     if(!instance) 
     {
	  int num_consoles = 1;
#ifdef CHICKEN_CONSOLE
#if CHICKEN_CONSOLE > 0
	  num_consoles++;
#endif
#endif
#ifdef PYTHON_CONSOLE
#if PYTHON_CONSOLE > 0
	  num_consoles++;
#endif
#endif
	  static Console console(num_consoles);
	  instance = &console;
     }
     
     return instance;
}

int *Console::requestIoFd (void)
{
     int i; ConsoleIO *cons;
     for (i = 0; i < nsessions; i++)
     {
	  cons = cios[i];
	  if (!(cons->isRequested()))
	  {
	       int *fd = new int[3];
	       fd[0] = i;
	       fd[1] = cons->getInputFd();
	       fd[2] = cons->getOutputFd();
	       return fd;
	  }
     }

     return NULL;
}

void Console::setTabLabel (unsigned int i, char *label)
{
     qon->setTabLabel (i,label);
}
