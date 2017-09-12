#include "hw1.h"
#include <stdlib.h>

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
int encryptPolybius(int row, int column, const char *key);
int decryptPolybius(int row ,int column, const char *key);
int encryptFmorse(const char *key);
int decryptFmorse(const char *key);
int isFmAlphabet(int argc, char **argv);
int isPolyAlphabet(int argc, char **argv);
int isValidKey(const char* key, char cipher);
int compare(int j, int k, char* buffer);
int strCompare(char *input, char *eof);
int size(int subscript);

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
    //printf("Good \n");
    //printf("%c \n",**(argv) );
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
    printf("hi \n");
    int row = 0;
    int column = 0;
    extern const char* key;

    key = NULL;


    if(argc>9)
        return 0;

    if(argc==3){
        row = 10;
        column = 10;
    }

   // printf("%d\n", argc);
   // printf("yo \n");

    if(argc>3){
       // printf("why its in\n");
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
                    //printf("%d\n", row);
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
                   // printf("%d\n", column);
            }
        }

        if(*(*(argv+j)+1)=='k'){ //Key
            if(argc==j+1)
                return 0;
            else{ //Key exists on cmd line

                key = *((argv+j+1));

                if(isValidKey(key, 'p')==0)
                    return 0;

                //printf("%s\n", key);
            }
        }

        }

        /*Checking key validity
        if(key!=NULL){
            if(isValidKey(key, 'p')==0)
            return 0;
        }*/

        if(row<9 || row>15 || column<9 || column>15 ||row*column<93) //Checking row and column validity
            return 0;
    }



    if(*(*(argv+2)) != '-' )
        return 0;

    if (*(*(argv+2)+1) == 'e'){
        encryptPolybius(row, column, key);
        printf("%d\n",(row<<4)|column );
        return ((row<<4)|column);
    }
    if (*(*(argv+2)+1) == 'd'){
        decryptPolybius(row, column, key);
        return (1<<13|((row<<4)|column));
    }

    return 0;


}

int fmorse(int argc, char **argv){

    extern const char* key;
    key = NULL;

    if(argc>5)
        return 0;

    if(argc<=2)
        return 0;

    if(*(*(argv+2)) != '-' )
        return 0;

    if(argc>3){
        if (*(*(argv+3)) != '-')
        return 0;
        if (*(*(argv+3)+1) != 'k')
        return 0;

        key = *((argv+4));

        printf("%s\n", key);
    }

    /*Checking key validity*/
        if(isValidKey(key, 'f')==0)
            return 0;

    if(*(*(argv+2)+1) == 'e'){
            encryptFmorse(key);
            return 0x4000;
        }
    if(*(*(argv+2)+1) == 'd'){
        decryptFmorse(key);
        return 0x6000;
    }

    return 0;

}
/*Validate the key*/
int isValidKey(const char* key, char cipher){

    for (int i = 0;*(key+i)!='\0' ; i++)
    {
        for (int j = i+1; *(key+j)!='\0'; j++)
        {
            if(*(key+i)==*(key+j))
                return 0;
        }

    }

    if(cipher=='p'){ //Polybius cipher
        for (int i = 0;*(key+i)!='\0' ; i++){
            if(*(key+i)<33 || *(key+i)>126)
                return 0;
        }
    }

    if(cipher=='f'){ //F Morse Cipher
        for (int i = 0;*(key+i)!='\0' ; i++){
            if(*(key+i)<'A' || *(key+i)>'Z')
                return 0;
        }

    }

    return 1;
}


int encryptPolybius(int row ,int column, const char *key){

    extern char polybius_table[];
    extern char *polybius_alphabet;
    printf("%d\n", row);
    printf("%d\n", column);

    int i=0;
        if(key!=NULL){ //Check if key exists
            printf("wrong\n");
            for(int j=0;*(key+j)!='\0';j++) //Storing the key if any in the table
                polybius_table[i++] = *(key+j);

            printf("wrong1\n");
            for(int j=0; j<=93;j++){

                int c=0;
                for(int k=0;*(key+k)!='\0';k++){
                    if (*(key+k) == *(polybius_alphabet+j)){
                    //printf("%c\n", *(polybius_alphabet+j));
                        c = 1;
                    }
                }//end of k

                if(c!=1){
                    polybius_table[i++] = *(polybius_alphabet+j);

                }
            }
        }
        else{
            for(i=0;*(polybius_alphabet+i)!='\0';i++)
                polybius_table[i] = *(polybius_alphabet+i);
        }

        //Fill in all remaining places with null character
    for(int j=i;j<row*column;j++)
        polybius_table[j] = '\0';



    int ch;
    while((ch = getchar())!= EOF){
            int c = 0;
            if(ch == 32 || ch == 9 || ch == 10){
                putchar(ch);
                c=1;
                continue;
            }
            for(int j=0;j<row*column; j++){

                if (polybius_table[j] == ch)
                {
                    c = 1;
                    if(j/column == 10)
                        printf("%c", 'A');
                    else if(j/column == 11)
                        printf("%c", 'B');
                    else if(j/column == 12)
                        printf("%c", 'C');
                    else if(j/column == 13)
                        printf("%c", 'D');
                    else if(j/column == 14)
                        printf("%c", 'E');
                    else if(j/column == 15)
                        printf("%c", 'F');
                    else
                        printf("%d",j/column);

                    if(j%column == 10)
                        printf("%c", 'A');
                    else if(j%column == 11)
                        printf("%c", 'B');
                    else if(j%column == 12)
                        printf("%c", 'C');
                    else if(j%column == 13)
                        printf("%c", 'D');
                    else if(j%column == 14)
                        printf("%c", 'E');
                    else if(j%column == 15)
                        printf("%c", 'F');
                    else
                        printf("%d",j%column);

                }

            }

            if(c!=1)
                return 0;
    }

    return 0;

}

int decryptPolybius(int row ,int column, const char *key){

    extern char polybius_table[];
    extern char *polybius_alphabet;
    //printf("%d\n", row);
    //printf("%d\n", column);

     int i=0;
        if(key!=NULL){ //Check if key exists
            printf("wrong\n");
            for(int j=0;*(key+j)!='\0';j++) //Storing the key if any in the table
                polybius_table[i++] = *(key+j);

            printf("wrong1\n");
            for(int j=0; j<=93;j++){

                int c=0;
                for(int k=0;*(key+k)!='\0';k++){
                    if (*(key+k) == *(polybius_alphabet+j)){
                    //printf("%c\n", *(polybius_alphabet+j));
                        c = 1;
                    }
                }//end of k

                if(c!=1){
                    polybius_table[i++] = *(polybius_alphabet+j);

                }
            }
        }
        else{
            for(i=0;*(polybius_alphabet+i)!='\0';i++)
                polybius_table[i] = *(polybius_alphabet+i);
        }

        //Fill in all remaining places with null character
    for(int j=i;j<row*column;j++)
        polybius_table[j] = '\0';




    int ch1, ch2;
    while((ch1 = getchar())!= EOF){

            if(ch1 == 32 || ch1 == 9 || ch1 == 10){
                putchar(ch1);
            }


            else{
            //GET CHAR BY PAIRS AND USE ROW MAJOR FORMULA
                int r, c;
                ch2 = getchar();
                if(ch1 == 'A')
                    r = 10;
                else if(ch1 == 'B')
                    r = 11;
                else if(ch1 == 'C')
                    r = 12;
                else if(ch1 == 'D')
                    r = 13;
                else if(ch1 == 'E')
                    r = 14;
                else if(ch1 == 'F')
                    r = 15;
                else
                    r = ch1-48;

                if(ch2 == 'A')
                    c = 10;
                else if(ch2 == 'B')
                    c = 11;
                else if(ch2 == 'C')
                    c = 12;
                else if(ch2 == 'D')
                    c = 13;
                else if(ch2 == 'E')
                    c = 14;
                else if(ch2 == 'F')
                    c = 15;
                else
                    c = ch2-48;

                int offset = (r*column)+c;
                printf("%c", polybius_table[offset]);
            }
    }

    return 0;

}

int encryptFmorse(const char *key){

    extern const char *morse_table[];
    extern char polybius_table[];
    extern char fm_key[];
    extern const char *fm_alphabet;
    extern const char *fractionated_table[];

    char *buffer;
    buffer = polybius_table ; //buffer CONTAINS MY INPUT IN MORSE CODE


    /*DETERMINE THE KEY*/
    int count=0;
    if(key!=NULL){
        for (int j = 0;*(key+j)!='\0'; ++j)
            fm_key[count++] = *(key+j);
    }
    //PUTTING REST OF THE ALPHABETS IN THE FM_KEY
    for(int j=0; j<=25;j++){

            int c=0;
            for(int k=0;*(key+k)!='\0';k++){
                if (*(key+k) == *(fm_alphabet+j)){
                    c = 1;
                }
            }//end of k

            if(c!=1){
                fm_key[count++] = *(fm_alphabet+j);
            }
        }

    int c, i = 0;
    while((c = getchar())!=EOF){ //ENCRYPTION

        if(c == 32|| c == 9){
            *(buffer+(i++)) = 'x';

        }
        else if(c == 10){ //WHEN NEWLINE IS ENCOUNTERED ENCRYPT THE STRING

            *(buffer+(i++)) = 'x';

            for(int i=0;*(buffer+i)!='\0';i=i+3){ //GET MORSE CODE FROM BUFFER INPUT IN GROUPS OF THREE AND CHECK WITH THE fractionated_table[]
            for(int j=0; j<26;j++){
                if(*(buffer+i) == *fractionated_table[j] && (*(buffer+i+1)) == (*(fractionated_table[j]+1)) && (*(buffer+i+2)) == (*(fractionated_table[j]+2))){
                    putchar(fm_key[j]);
                }
            }

        }
        putchar(10);
        i =0;
        continue;
        }
        else{
                if(*morse_table[c-33] == '\0')
                    return 1;
                for(int j=0;*(morse_table[c-33]+j)!='\0';j++){
                    *(buffer+(i++)) = *(morse_table[c-33]+j);

            }
            *(buffer+(i++)) = 'x';
        }

    }

    return 0;
}

int decryptFmorse(const char *key){

    extern const char *morse_table[];
    extern char polybius_table[];
    extern char fm_key[];
    extern const char *fm_alphabet;
    extern const char *fractionated_table[];

    /*DETERMINE THE KEY*/
    int count=0;
    if(key!=NULL){
        for (int j = 0;*(key+j)!='\0'; ++j)
            fm_key[count++] = *(key+j);
    }
    //PUTTING REST OF THE ALPHABETS IN THE FM_KEY
    for(int j=0; j<=25;j++){

            int c=0;
            for(int k=0;*(key+k)!='\0';k++){
                if (*(key+k) == *(fm_alphabet+j)){
                    c = 1;
                }
            }//end of k

            if(c!=1){
                fm_key[count++] = *(fm_alphabet+j);
            }
        }

    char *buffer;
    buffer = polybius_table ; //buffer CONTAINS MY INPUT IN MORSE CODE

    int c, j=0, flag =0;
    while((c = getchar())!=EOF){
        if(c == 10){ //THIS IS WHEN THE FULL FRACTIONED MORSE CODE IS IN THE BUFFER

           /*for(int i=0;*(buffer+i)!='\0';i++)
                printf("%c", *(buffer+i));
            printf("\n");*/
            int k=0;
            for (int i = 0; *(buffer+i)!='\0'; i++)
            {   //printf("%c\n", *(buffer+i));
                if(flag == 1 && *(buffer+i)=='x'){
                    //printf("flag %d\n", flag);
                    //printf("%d", flag);
                    k = i+1;
                    putchar(32);
                    flag =0;
                    continue;
                }
                if(*(buffer+i)=='x'){ //GET ALL THE CHARACTERS BEFORE x
                    //printf("In \n");
                    if(*(buffer+i+1)=='x')
                        flag = 1;
                    else
                        flag =0;
                    for(int j=0;j<89;j++){
                        //printf("%d\n", j);
                        //printf("%ld\n",sizeof(*(morse_table[j])) );
                        if((compare(j, k, buffer) == 1) && (*(morse_table[j])!='\0') && (i-k) == size(j)){
                            //printf("%s\n", (morse_table[j]));
                            //printf("%d \n",j);
                            putchar(j+33);
                            k = i+1;
                            break;

                        }
                    }
                }
                //printf("i = %d\n", i);
            }

            j = 0;
            printf("\n");
        }
        else{
            for(int i=0;i<27;i++){
                if(fm_key[i] == c){
                    //printf("%d \n", i);
                    //printf("%c \n", fm_key[i]);
                    *(buffer+(j++)) = *fractionated_table[i];
                    *(buffer+(j++)) = *(fractionated_table[i]+1);
                    *(buffer+(j++)) = *(fractionated_table[i]+2);
                }
            }

        }
    }

    return 0;
}

int size(int subscript){
    extern const char *morse_table[];

    int s=0;
    for(int i=0; *(morse_table[subscript]+i) !='\0';i++)
        s++;

    return s;
}

int compare(int j, int k, char* buffer){

    extern const char *morse_table[];

    int flag =0;
    for(int i=0; *(morse_table[j]+i) !='\0';i++){
        //printf("%c", *(buffer+k+i));
        //printf("%d \n", j);
        if(*(buffer+k+i) != *(morse_table[j]+i)){
            flag =1;
            break;
        }
    }



    if(flag ==0)
        return 1;

    return 0;
}

int strCompare(char *input, char *eof){

    int i=0;
    int flag = 0;


    while(*(input+i)!='\0' && *(eof+i)!='\0'){
        if (*(input+i)!=*(eof+i))
        {
            flag = 1;
            break;
        }
        i++;
    }

    if(flag == 0 && *(input+i)=='\0' && *(eof+i)=='\0')
        return 1;
    return 0;
}
