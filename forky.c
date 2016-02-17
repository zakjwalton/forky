#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define SIZE 50
char* stack;
int top=-1;       /* Global declarations */
//
//   Your other functions
//   and variables
//
//   should go here
//
void push(char elem)
{                       /* Function for PUSH operation */
    stack[++top]=elem;
}

char pop()
{                      /* Function for POP operation */
    return(stack[top--]);
}

int pr(char elem)
{                 /* Function for precedence */
    switch(elem)
    {
    case '#': return 0;
    case ')': return 1;
    case '+':
    case '-': return 2;
    case '*':
    case '/': return 3;
    }
    return 0;
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

void infixToPrefix(char* infx)
{                         /* Main Program */
    char prfx[50],ch,elem;
    char** tokString;
    char* token;
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
    printf("Number of elements is: %d\n",numElements);

    //Malloc array to store each each expression element as well as operator stack
    tokString = (char **)malloc(numElements*sizeof(char*));
    stack = (char *)malloc(numElements*sizeof(char));

    //Split infix string up into an array of string elements
    token = strtok(infx, " ");
    i = 0;
    tokString[i] = token;
    while((token = strtok(NULL, " ")))
    {
       i++;
       tokString[i] = token;
    }

    for( i = 0 ; i < numElements ; i++)
    {
        printf("%s\n",tokString[i]);
    }

    //Push # on stack to indicate first operator
    push('#');

    //Reverse array of string elements
    tokString = reverseArray(tokString, numElements);

    for( i = 0 ; i < numElements ; i++)
    {
        printf("%s\n",tokString[i]);
    }


    return;

    /*
    push('#');
    strrev(infx);
    while( (ch=infx[i++]) != '\0')
    {
        if( ch == ')') push(ch);
        else
            if(isalnum(ch)) prfx[k++]=ch;
            else
                if( ch == '(')
                {
                    while( stack[top] != ')')
                        prfx[k++]=pop();
                    elem=pop(); // Remove )
                }
                else
                {       // Operator
                    while( pr(stack[top]) >= pr(ch) )
                        prfx[k++]=pop();
                    push(ch);
                }
    }
    while( stack[top] != '#')     // Pop from stack till empty
        prfx[k++]=pop();
    prfx[k]='\0';          // Make prfx as valid string
    strrev(prfx);
    strrev(infx);
    printf("\n\nGiven Infix Expn: %s  Prefix Expn: %s\n",infx,prfx);
    */
}

float evaluate (const char* expr, bool immediate_result)
{
    char inputStr[strlen(expr)+1];
    //Copy in input string
    strcpy(inputStr, expr);
    //Convert input string to prefix
    infixToPrefix(inputStr);

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
    printf ("Test 4 %f\n", evaluate ("16.0 - 10.5 - 12.3 + 54.2", true));
    /* add more test of your own */
    /* 8< 8< 8< end-cut-here >8 >8 >8 */
    return 0;
}


