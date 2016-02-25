/*
 * Parallel evaluation of arithmetic expressions
 * Program: forky.c
 * Class: CIS 452
 * Instructor: Dr. Dulimarta
 * Author: Zak Walton
 */

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

//Macros
//#define DEBUG

#define SIZE 50

#define READ 0
#define WRITE 1


//Type for each node in the expression tree
typedef struct element_s{
    char operator;
    float value;
    struct element_s* leftChild;
    struct element_s* rightChild;
}element_t;

//Globals related to tree construction
char** stack;
int top = -1;
element_t* treeTop;

bool noPause = false;
bool sigIntFlag = false;
bool sigUsrFlag = false;

/*
 * Signal Handlers
 */
void sigIntHandler(int signum)
{
    sigIntFlag = true;
}
void sigUsrHandler(int signum)
{
    sigUsrFlag = true;
}

/*
 * Function that prints out an array of strings.
 */
void printCharStarStar(char** ary, int size)
{
    int i;
    for( i = 0 ; i < size ; i++)
    {
        printf("%s ",ary[i]);
    }
    printf("\n");
}

/*
 * Function to push an operator on to the top of the stack
 */
void push(char* elem)
{                       /* Function for PUSH operation */
    stack[++top]=elem;
}

/*
 * Pop an operator off the top of the stack
 */
char* pop()
{                      /* Function for POP operation */
    return(stack[top--]);
}

/*
 * Evaluate the precedence of the element.  Return 0 if the element isn't an
 * operator.
 */
int pr(char* elem)
{                 /* Function for precedence */
    switch(elem[0])
    {
    case '#':
        return 0;
    case '-':
    case '+':
        return 1;
    case '/':
    case '*':
        return 2;
    default:
        return 3;
    }
}

/*
 * Reverse an array of strings
 */
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

/*
 * Convert an infix string to an array of strings in prefix
 */
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
    tokString = (char **)malloc((numElements+10)*sizeof(char*));
    prfx = (char **)malloc((numElements+10)*sizeof(char*));
    stack = (char **)malloc((numElements+10)*sizeof(char*));

    //Split infix string up into an array of string elements
    token = strtok(infx, " ");
    i = 0;
    tokString[i] = token;
    while((token = strtok(NULL, " \n")))
    {
        i++;
        tokString[i] = token;
    }

    //Print the original expression
    printf("\nHere is the infix version\n");
    printCharStarStar(tokString, numElements);

    //Reverse array of string elements
    tokString = reverseArray(tokString, numElements);

#ifdef DEBUG
    printCharStarStar(tokString, numElements);
#endif

    //Push # on stack to indicate first operator
    push("# ");
    i = 0;
    k = 0;
    while(i < numElements )
    {
        ch = tokString[i];
        i++;
        if(pr(ch) == 3)
        {
            prfx[k++]=ch;
        }
        else
        {
            while( pr(stack[top]) > pr(ch) )
                prfx[k++]=pop();
            push(ch);
        }
    }
    // Pop from stack till empty
    while(pr(stack[top]) != 0)
    {
        prfx[k++] = pop();
    }
    prfx = reverseArray(prfx, numElements);
    tokString = reverseArray(tokString, numElements);
    *size = numElements;

    //Print prefix version
    printf("Here is the prefix version\n");
    printCharStarStar(prfx, numElements);

    free(tokString);
    free(stack);
    return prfx;
}

/*
 * Create a tree of the expression to be evaluated, given an
 * array of strings representing the arithmetic expression in
 * prefix format as well as a pointer to the top node pointer.
 */
char** createTree(char** prfx, element_t** node)
{
    char** temp = NULL;
    //Allocate memory for this node
    *node = (element_t*)malloc(sizeof(element_t));
    if(pr(prfx[0]) == 3)
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

/*
 * Print the whole tree out given a pointer to the top node.
 */
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

/*
 * Free the whole tree starting from the top
 */
void freeTree(element_t* node)
{
    if(node->operator == '=')
    {
        free(node);
    }
    else
    {
        freeTree(node->rightChild);
        freeTree(node->leftChild);
        free(node);
    }
    return;
}

/*
 * Function that is called recursively to evaluate the expression tree.
 */
float evalExpression(element_t* node)
{
    float resultLeft;
    float resultRight;
    bool noLeft, noRight;
    element_t* leftNode = node->leftChild;
    element_t* rightNode = node->rightChild;
    int fdl[2];
    int fdr[2];
    int statusl;
    int statusr;
    pid_t pidl, wpidl;
    pid_t pidr, wpidr;

    printf("I'm a child with pid->%d, and my operator is: %c\n",getpid(),node->operator);

    //Check left, recurse if necessary
    if(leftNode->operator == '=')
    {
        //Left node is not operator, extract value
        resultLeft = leftNode->value;
        noLeft = true;
    }
    else
    {
        //Left node is operator, fork and recurse

        noLeft = false;
        //Create pipe
        if(pipe(fdl) < 0)
        {
            perror("Problem creating the pipe");
            exit(1);
        }

        //Fork for top operator
        if((pidl = fork()) < 0)
        {
            perror("fork failed");
            exit(1);
        }
        else if(pidl == 0) // Child
        {
            //Close read end of pipe
            close(fdl[READ]);
            //Evaluate expression
            resultLeft = evalExpression(leftNode);
            //Write result to pipe
            write(fdl[WRITE], (const void *)&resultLeft, (size_t) sizeof(float));
            close(fdl[WRITE]);
            //Exit
            exit(0);
        }
        else // Parent
        {
            //Close write end of the pipe
            close(fdl[WRITE]);
            printf("I'm also a parent with pid->%d and my left child has pid->%d\n",getpid(),pidl);
        }
    }

    //Check right, recurse if necessary
    if(rightNode->operator == '=')
    {
        //Left node is not operator, extract value
        resultRight = rightNode->value;
        noRight = true;
    }
    else
    {
        //Left node is operator, fork and recurse
        noRight = false;
        //Create pipe
        if(pipe(fdr) < 0)
        {
            perror("Problem creating the pipe");
            exit(1);
        }

        //Fork for top operator
        if((pidr = fork()) < 0)
        {
            perror("fork failed");
            exit(1);
        }
        else if(pidr == 0) // Child
        {
            //Close read end of pipe
            close(fdr[READ]);
            //Evaluate expression
            resultRight = evalExpression(rightNode);
            //Write result to pipe
            write(fdr[WRITE], (const void *)&resultRight, (size_t) sizeof(float));
            close(fdr[WRITE]);
            //Exit
            exit(0);
        }
        else // Parent
        {
            //Close write end of the pipe
            close(fdr[WRITE]);
            printf("I'm also a parent with pid->%d and my right child has pid->%d\n",getpid(),pidr);
        }
    }

    //Wait for SIGUSR1 from parent to issue signal to child
    if(!noPause)
    {
        sigUsrFlag = false;
        while(sigUsrFlag == false)
        {
            pause();
        }
        if(!noLeft)
        {
            kill(pidl, SIGUSR1);
        }
        if(!noRight)
        {
            kill(pidr, SIGUSR1);
        }
    }

    //wait on left
    if(!noLeft)
    {
        wpidl = waitpid(pidl, &statusl, 0);
        if(wpidl < 0)
        {
            perror("Error while waiting");
            exit(1);
        }
        //Read left result
        read(fdl[READ], (void *)&resultLeft, (size_t) sizeof(float));
        close(fdl[READ]);
#ifdef DEBUG
        printf("Result is %f\n",resultLeft);
#endif
    }

    //wait on right
    if(!noRight)
    {
        wpidr = waitpid(pidr, &statusr, 0);
        if(wpidr < 0)
        {
            perror("Error while waiting");
            exit(1);
        }
        //Read right result
        read(fdr[READ], (void *)&resultRight, (size_t) sizeof(float));
        close(fdr[READ]);
#ifdef DEBUG
        printf("Result is %f\n",resultRight);
#endif
    }

    //Return case based on operator
    switch(node->operator)
    {
    case '/':
        freeTree(treeTop);
        printf("My pid->%d and I'm returning %f\n",getpid(),resultLeft/resultRight);
        return resultLeft / resultRight;
    case '*':
        freeTree(treeTop);
        printf("My pid->%d and I'm returning %f\n",getpid(),resultLeft*resultRight);
        return resultLeft * resultRight;
    case '-':
        freeTree(treeTop);
        printf("My pid->%d and I'm returning %f\n",getpid(),resultLeft-resultRight);
        return resultLeft - resultRight;
    case '+':
        freeTree(treeTop);
        printf("My pid->%d and I'm returning %f\n",getpid(),resultLeft+resultRight);
        return resultLeft + resultRight;
    default:
        printf("Error, invalid operator\n");
        return 0;
    }
}

/*
 * Main evaluate function.  Takes in an infix expression and whether or not to
 * evaluate the expression immediately or not.
 */
float evaluate(const char* expr, bool immediate_result)
{
    char** prefix;
    char inputStr[strlen(expr)+1];
    int size;
    int status;
    int fd[2];
    float result;
    pid_t pid, wpid;

    //set pause
    noPause = immediate_result;

    //register signals
    signal(SIGINT, sigIntHandler);
    signal(SIGUSR1, sigUsrHandler);

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

    //No operators in the tree
    if(treeTop->operator == '=')
    {
        freeTree(treeTop);
        return treeTop->value;
    }
    else
    {
        //Create pipe
        if(pipe(fd) < 0)
        {
            perror("Problem creating the pipe");
            exit(1);
        }

        //Fork for top operator
        if((pid = fork()) < 0)
        {
            perror("fork failed");
            exit(1);
        }
        else if(pid == 0) // Child
        {
            //Close read end of pipe
            close(fd[READ]);
            //Evaluate expression
            result = evalExpression(treeTop);
            //Write result to pipe
            write(fd[WRITE], (const void *)&result, (size_t) sizeof(float));
            close(fd[WRITE]);
            //Wait for signal from parent to terminate
            exit(0);
        }
        else // Parent
        {
            //Close write end of the pipe
            close(fd[WRITE]);
            //Print parent and child relationship
            printf("I'm the top level parent with pid->%d and I spawned a child with pid->%d\n",getpid(), pid);

            //Wait on SIGINT, then notify child if pause enabled
            if(!noPause)
            {
                sigIntFlag = false;
                while(sigIntFlag == false)
                {
                    pause();
                }
                kill(pid, SIGUSR1);
            }

            //Wait for child to exit
            wpid = waitpid(pid, &status, 0);
            if(wpid < 0)
            {
                perror("Error while waiting");
                exit(1);
            }

            //Read in result from pipe
            read(fd[READ], (void *)&result, (size_t) sizeof(float));
            close(fd[READ]);
#ifdef DEBUG
            printf("Result is %f\n",result);
#endif
        }
    }

    //Free tree structure
    freeTree(treeTop);
    return result;
}


int main()
{

    /* 8< 8< 8< begin-cut-here >8 >8 >8 */
    printf ("Test 1 %f\n", evaluate ("2.0 + 3.0 + 4.0 + 5.0", false));
    printf ("Test 2 %f\n", evaluate ("2.0 + 3.0 + 4.0 + 5.0", false));
    printf ("Test 3 %f\n", evaluate ("200.0 + 300.0", false));
    printf ("Test 4 %f\n", evaluate ("2.0 * 3.0 + 4.0 / 5.0", false));
    printf ("Test 5 %f\n", evaluate ("2.0 + 3.0 * 4.0 - 5.0 / 6.0", false));
    printf ("Test 5 %f\n", evaluate ("2.0 + 15.0", false));
    /* add more test of your own */
    /* 8< 8< 8< end-cut-here >8 >8 >8 */
    return 0;
}


