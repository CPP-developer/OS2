#include <sys/wait.h> 
#include<string> 
#include<cstring> 
#include<sstream> 
#include<string> 
#include <stdlib.h> 
#include<iostream> 
#include <stdio.h> 
#include <unistd.h> 
#include <assert.h> 
#include <sys/types.h>

using namespace std; 

int main () {
	int output_fd[2];
	int input_fd[2];
	int rezultat, i;
	srand(time(0));
	istringstream string_stream;
	string aritmeticki_izraz="";
	string text_za_prijenos="";
	
	if (pipe(output_fd) == -1)
	exit(1);
	if (pipe(input_fd) == -1)
	exit(1);
	
	switch (fork()) {

		case -1: 

				exit(1);

		case 0:
					
					close(input_fd[1]);
					
					dup2 (input_fd[0],0); 
					
					
					close(output_fd[0]);
			
					dup2 (output_fd[1],1); 
					dup2 (output_fd[1],2); 
					
					
					
					
					
					execl("/usr/bin/bc","bc", NULL);
					exit(0);

		default:
				
				char buffer[100];
				cout<<endl<<endl;
				for(int j=0; j<5;j++)
					{
					string_stream.str("");
					srand(time(0));
					rezultat=rand()%100;
					
					cout << "\033[1;31m" <<rezultat << "= " <<"\033[0m";
					
					


					cin>>aritmeticki_izraz;
					
					string_stream<<aritmeticki_izraz<<"\n";
					text_za_prijenos = string_stream.str();
					
					close(input_fd[0]);
					write(input_fd[1],&text_za_prijenos[0],text_za_prijenos.length());
				
					close(output_fd[1]);
					read(output_fd[0],buffer,100);
					
					//cout << input <<endl;
					i=atoi(buffer);
					//cout << i <<endl;
					
					
					if(i==rezultat)
					{
						cout << "\033[1;34m" <<"ISPRAVNO" <<"\033[0m\n"<<endl;
					}
					else if(strncmp(buffer, "(standard_in)",11) == 0)
					{
						cout << "\033[1;34m" <<"NEISPRAVAN IZRAZ" <<"\033[0m\n"<<endl;
					}
					else{
						
						cout << "\033[1;34m" <<"NEISPRAVNO, točan odgovor je: "<<i <<"\033[0m\n"<<endl;
					}
				}
					
					
					
					
				close(output_fd[0]);
				close(input_fd[1]);
				wait(NULL);
				exit(0);
				

					}
exit(0);

}


