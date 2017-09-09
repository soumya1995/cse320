#include "hw1.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

/* function declaration */
int polybius(int argc, char **argv);
int fmorse(int argc, char **argv);
int encryptpolybius(int argc, char **argv);
int decryptpolybius(int argc, char **argv);
int encryptfmorse(int argc, char **argv);
int decryptfmorse(int argc, char **argv);
int isRepeat(int argc, char **argv);
int isFmAlphabet(int argc, char **argv);
int isPolyAlphabet(int argc, char **argv);
int isValidKey(int argc, char **argv, char cipher);

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the program
 * and will return a unsigned short (2 bytes) that will contain the
 * information necessary for the proper execution of the program.
 *
 * IF -p is given but no (-r) ROWS or (-c) COLUMNS are specified this function
 * MUST set the lower bits to the default value of 10. If one or the other
 * (rows/columns) is specified then you MUST keep that value rather than assigning the default.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return Refer to homework document for the return value of this function.
 */

unsigned short validargs(int argc, char **argv) {
    printf("Good \n");
    printf("%c \n",**(argv) );
    //printf("%c \n",**(argv+1) );
    //printf("%c \n", *(*(argv+1)+1) );

    if(argc<=1)
        return 0;

    if(*(*(argv+1)) != '-' )
        return 0;


    if(*(*(argv+1)+1) == 'h')
        return 0x8000;
    else if (*(*(argv+1)+1) == 'p')
       return polybius(argc, argv);
    else if (*(*(argv+1)+1) == 'f')
       return fmorse(argc, argv);
    else
        return 0;

}

int polybius(int argc, char **argv){

    int row = 0;
    int column = 0;
    char* key = "my";



    if(argc>9)
        return 0;

    if(argc==3){
        row = 10;
        column = 10;
    }

    if(argc>3){
        for(int j=3;j<argc;j=j+2){


        if(*(*(argv+j))!='-')
            return 0;
        if(*(*(argv+j)+1)=='r'){ //Checking for rows
            if(argc==j+1)
                return 0;
            else {//Row number exits on cmd line
                for(int i=0;*(*(argv+j+1)+i)!='\0';i++){
                    if(i>=2) //size of row is over 2 digits
                        return 0;
                    row = row*10+(*(*(argv+j+1)+i)-48);
                }
                    printf("%d\n", row);
            }
        }

        if(*(*(argv+j)+1)=='c'){ //Checking columns
            if(argc==j+1)
                return 0;
            else {//Column number exits on cmd line
                for(int i=0;*(*(argv+j+1)+i)!='\0';i++){
                    if(i>=2) //size of column is over 2 digits
                        return 0;
                    column = column*10+(*(*(argv+j+1)+i)-48);
                }
                    printf("%d\n", column);
            }
        }

        if(*(*(argv+j)+1)=='k'){ //Key
            printf("%s\n", "yo" );
            if(argc==j+1)
                return 0;
            else{ //Key exists on cmd line
                for (int i=0;*(*(argv+j+1)+i)!='\0';i++){
                    printf("%d\n", i);
                    printf("%c\n", *(*(argv+j+1)+i));
                    *(key+i) = *(*(argv+j+1)+i);

                }
                    printf("%s\n", "coming");
                    printf("%s\n", key);
            }
        }

        }

        if(row<9 || row>15 || column<9 || column>15)
            return 0;
    }

    if(*(*(argv+2)) != '-' )
        return 0;

  /*  if (*(*(argv+2)+1) == 'e')
        encryptpolybius(argc, argv);
    if (*(*(argv+2)+1) == 'd')
        decryptpolybius(argc, argv);*/

    return 0;


}

int fmorse(int argc, char **argv){

    if(argc<=2)
        return 0;

    if(*(*(argv+2)) != '-' )
        return 0;

   /* if(*(*(argv+2)+1) != 'e')
        encryptfmorse(argc, argv);
    if(*(*(argv+2)+1) != 'd')
        decryptfmorse(argc, argv);*/

    /*if (*(*(argv+2)+1) == 'd'){
       if(isValidKey(argc, argv, 'f') ==1)
            return 0x6000;
        else
            return 0;
    }

    else if (*(*(argv+2)+1) == 'e' && argc>3){
        if(isValidKey(argc, argv, 'f')==1)
            return 0x4000;
        else
            return 0;
    }
    else
        return 0;*/
    return 0;

}

/*This function will check if a key exists. If a key exists we validate it.
  @argc is length of command line argument
  @argv is the pointer to the line arument*/
int isValidKey(int argc, char **argv, char cipher){
    printf("%d\n", argc);

    if(*(*(argv+3)) != '-' )
        return 0;

    if(*(*(argv+3)+1)!='k')
        return 0;

    if(cipher == 'p'){
        if(isRepeat(argc, argv)!=1 /*&& isFmAlphabet(argc, argv)==1*/)
            return 1;
    }

    if(cipher == 'f'){
        if(isRepeat(argc, argv)!=1 /*&& isPolyAlphabet(argc, argv)==1*/)
            return 1;
    }

    return 0;
}

int isRepeat(int argc, char **argv){

    int i=0;

    printf("cool \n");
    printf("%c\n", *(*(argv+4)+4));

    if(*(*(argv+4)+3) == '\0')
        printf("success");

    if(*(*(argv+4)+3) == *(*(argv+5)))
        printf("success" );

    while(*(*(argv+3)+i)){

    }

    return 1;
}

int encryptpolybius(int argc, char **argv){
    return 0;

}
