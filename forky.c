#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

//#define DEBUG
#define DEBUG

#define SIZE 50
char** stack;
int top=-1;       /* Global declarations */
//
//   Your other functions
//   and variables
//
//   should go here
//
void push(char* elem)
{                       /* Function for PUSH operation */
    stack[++top]=elem;
}

char* pop()
{                      /* Function for POP operation */
    return(stack[top--]);
}

int pr(char* elem)
{                 /* Function for precedence */
    switch(elem[0])
    {
    case '-':
        return 1;
    case '+':
        return 2;
    case '/':
        return 3;
    case '*':
        return 4;
    default:
        return 0;
    }
}

char** reverseArray(char** input, int length)
{
    char** revInput = (char**)malloc(length*sizeof(char*));
    int i,k;
    for(i = 0, k = length-1 ; i < length ; i++, k--)
    {
        revInput[k] = input[i];
    }

    free(input);
    return revInput;

}

char** infixToPrefix(char* infx, int* size)
{                         /* Main Program */
    char** tokString;
    char** prfx;
    char* token;
    char* ch;
    int i=0,k=0;
    int numElements = 0;

    //Count number of elements in the expression (operators and number)
    for( i = 0 ; i < strlen(infx) ; i++ )
    {
        if(infx[i] == ' ')
        {
            numElements++;
        }
    }
    numElements++;
#ifdef DEBUG
    printf("\nNumber of elements is: %d\n",numElements);
#endif

    //Malloc array to store each each expression element as well as operator stack
    tokString = (char **)malloc(numElements*sizeof(char*));
    prfx = (char **)malloc(numElements*sizeof(char*));
    stack = (char **)malloc(numElements*sizeof(char*));

    //Split infix string up into an array of string elements
    token = strtok(infx, " ");
    i = 0;
    tokString[i] = token;
    while((token = strtok(NULL, " \n")))
    {
        i++;
        tokString[i] = token;
    }

#ifdef DEBUG
    for( i = 0 ; i < numElements ; i++)
    {
        printf("%s ",tokString[i]);
    }
    printf("\n");
#endif

    //Push # on stack to indicate first operator
    push("#");

    //Reverse array of string elements
    tokString = reverseArray(tokString, numElements);

#ifdef DEBUG
    for( i = 0 ; i < numElements ; i++)
    {
        printf("%s ",tokString[i]);
    }
    printf("\n");
#endif

    i = 0;
    k = 0;
    while(i < numElements )
    {
        ch = tokString[i];
        i++;
        if(!pr(ch))
        {
            prfx[k++]=ch;
        }
        else
        {
            while( pr(stack[top]) >= pr(ch) )
                prfx[k++]=pop();
            push(ch);
        }
    }
    // Pop from stack till empty
    while( pr(stack[top]))
    {
        prfx[k++]=pop();
    }
    prfx = reverseArray(prfx, numElements);
    tokString = reverseArray(tokString, numElements);
    *size = numElements;

#ifdef DEBUG
    printf("Here is the infix version\n");
    for( i = 0 ; i < numElements ; i++)
    {
        printf("%s ",prfx[i]);
    }
    printf("\n");
#endif
    free(tokString);
    free(stack);
    return prfx;
}

float evaluate (const char* expr, bool immediate_result)
{
    char** prefix;
    char inputStr[strlen(expr)+1];
    int size;
    //Copy in input string
    strcpy(inputStr, expr);
    //Convert input string to prefix
    prefix = infixToPrefix(inputStr, &size);

    return 0.0;  /* replace it with your own */
}

int main()
{

    //
    // you may add more code here
    //

    /* 8< 8< 8< begin-cut-here >8 >8 >8 */
    printf ("Test 1 %f\n", evaluate ("2.0 * 3.0", true));
    printf ("Test 2 %f\n", evaluate ("200.0 + 300.0", true));
    printf ("Test 3 %f\n", evaluate ("10.0 / 5.0", true));
    printf ("Test 4 %f\n", evaluate ("2.0 * 3.0 + 4.0 / 5.0", true));
    /* add more test of your own */
    /* 8< 8< 8< end-cut-here >8 >8 >8 */
    return 0;
}


