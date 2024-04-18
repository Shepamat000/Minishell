# Minishell
 A shell program for Unix systems in C

 Run the following commands in a Linux environment with Git installed

 ```
git clone --recurse-submodules https://github.com/Shepamat000/Minishell
cd ./Minishell
cd ./Minishell
make
./msh
```

The Minishell current supports piping, input redirection, output redirection, standard error redirection, and background commands. 

Unique internal commands include mycalc, which follows for format mycalc <num1> <add/mul/div> <num2> and myhistory, which lists the history of the current shell process.  Myhistory can also be used with an index to run that command, such as myhistory 0 to run the 1st command in the history.  

