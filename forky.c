/*
 * Parallel evaluation of arithmetic expressions
 * Program: forky.c
 * Class: CIS 452
 * Instructor: Dr. Dulimarta
 * Author: Zak Walton
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define DEBUG
#define SIZE 50

//Type for each node in the expression tree
typedef struct element_s{
    char operator;
    float value;
    struct element_s* leftChild;
    struct element_s* rightChild;
}element_t;

char** stack;
int top=-1;
element_t* treeTop;


void printCharStarStar(char** ary, int size)
{
    int i;
    for( i = 0 ; i < size ; i++)
    {
        printf("%s ",ary[i]);
    }
    printf("\n");
}

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
    printCharStarStar(tokString, numElements);
#endif

    //Push # on stack to indicate first operator
    push("#");

    //Reverse array of string elements
    tokString = reverseArray(tokString, numElements);

#ifdef DEBUG
    printCharStarStar(tokString, numElements);
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
    printCharStarStar(prfx, numElements);
#endif
    free(tokString);
    free(stack);
    return prfx;
}

char** createTree(char** prfx, element_t** node)
{
    char** temp = NULL;
    //Allocate memory for this node
    *node = (element_t*)malloc(sizeof(element_t));
    if(!pr(prfx[0]))
    {
        //If it's a number, parse the float and return the next prefix element
        (*node)->operator = '=';
        (*node)->value = atof(prfx[0]);
        (*node)->rightChild = NULL;
        (*node)->leftChild = NULL;
        return &prfx[1];
    }
    else
    {
        //If it's an operator, set the operator and recurse to create the left
        //and right branches
        (*node)->operator = prfx[0][0];
        (*node)->value = -1;
        temp = createTree(&prfx[1], &(*node)->leftChild);
        return createTree(temp, &(*node)->rightChild);
    }
}

//print tree
void printTree(element_t* node)
{
    if(node->value >= 0)
    {
        printf("I'm a value: %f\n",node->value);
    }
    else
    {
        printf("I'm an operator: %c\n",node->operator);
        printTree(node->leftChild);
        printTree(node->rightChild);
    }
}

float evaluate(const char* expr, bool immediate_result)
{
    char** prefix;
    char inputStr[strlen(expr)+1];
    int size;
    //Copy in input string
    strcpy(inputStr, expr);
    //Convert input string to prefix
    prefix = infixToPrefix(inputStr, &size);
    //Construct tree from expression in prefix
    createTree(prefix, &treeTop);
    free(prefix);
#ifdef DEBUG
    printTree(treeTop);
#endif


    return 0.0;  /* replace it with your own */
}


int main()
{

    //
    // you may add more code here
    //

    /* 8< 8< 8< begin-cut-here >8 >8 >8 */
    printf ("Test 1 %f\n", evaluate ("2.0", true));
    printf ("Test 2 %f\n", evaluate ("200.0 + 300.0", true));
    printf ("Test 3 %f\n", evaluate ("10.0 / 5.0", true));
    printf ("Test 4 %f\n", evaluate ("2.0 + 3.0 * 4.0 - 5.0 / 6.0", true));
    /* add more test of your own */
    /* 8< 8< 8< end-cut-here >8 >8 >8 */
    return 0;
}


