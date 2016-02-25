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
    /*
    printf ("Test 1 %f\n", evaluate ("2.0 + 3.0 + 4.0 + 5.0", false));
    printf ("Test 2 %f\n", evaluate ("2.0 + 3.0 + 4.0 + 5.0", false));
    printf ("Test 3 %f\n", evaluate ("200.0 + 300.0", false));
    printf ("Test 4 %f\n", evaluate ("2.0 * 3.0 + 4.0 / 5.0", false));
    printf ("Test 5 %f\n", evaluate ("2.0 + 3.0 * 4.0 - 5.0 / 6.0", false));
    printf ("Test 5 %f\n", evaluate ("2.0 + 15.0", false));
    */
    /* add more test of your own */
    /* 8< 8< 8< end-cut-here >8 >8 >8 */
    // 1 operator 
    printf("Test  1 ===> %7.3f\n", evaluate("4.0 - 3.0", true));
    printf("Test  2 ===> %7.3f\n", evaluate("19.0 - 17.0", true));
    printf("Test  3 ===> %7.3f\n", evaluate("10.0 - 7.0", true));
    printf("Test  4 ===> %7.3f\n", evaluate("15.0 - 11.0", true));
    printf("Test  5 ===> %7.3f\n", evaluate("8.0 - 3.0", true));
    printf("Test  6 ===> %7.3f\n", evaluate("18.0 - 12.0", true));
    printf("Test  7 ===> %7.3f\n", evaluate("16.0 - 9.0", true));
    printf("Test  8 ===> %7.3f\n", evaluate("4.0 + 4.0", true));
    printf("Test  9 ===> %7.3f\n", evaluate("16.0 - 7.0", true));
    printf("Test 10 ===> %7.3f\n", evaluate("12.0 - 2.0", true));
    // 2 operators 
    printf("Test 11 ===> %7.3f\n", evaluate("73.3 + 9.0 - 71.3", true));
    printf("Test 12 ===> %7.3f\n", evaluate("114.0 / 1.0 - 102.0", true));
    printf("Test 13 ===> %7.3f\n", evaluate("129.0 + 17.0 - 133.0", true));
    printf("Test 14 ===> %7.3f\n", evaluate("33.3 - 15.0 - 4.3", true));
    printf("Test 15 ===> %7.3f\n", evaluate("58.4 + 6.0 - 49.4", true));
    printf("Test 16 ===> %7.3f\n", evaluate("82.1 * 2.0 - 148.2", true));
    printf("Test 17 ===> %7.3f\n", evaluate("90.4 + 19.0 - 92.4", true));
    printf("Test 18 ===> %7.3f\n", evaluate("35.2 / 10.0 + 14.48", true));
    printf("Test 19 ===> %7.3f\n", evaluate("133.5 + 10.0 - 124.5", true));
    printf("Test 20 ===> %7.3f\n", evaluate("145.3 / 19.0 + 12.3526315789", true));
    // 3 operators 
    printf("Test 21 ===> %7.3f\n", evaluate("23.1 * 40.8 + 17.0 - 938.48", true));
    printf("Test 22 ===> %7.3f\n", evaluate("179.9 * 188.9 / 6.0 - 5641.85166667", true));
    printf("Test 23 ===> %7.3f\n", evaluate("79.0 / 75.9 * 8.0 + 14.6732542819", true));
    printf("Test 24 ===> %7.3f\n", evaluate("118.2 * 163.0 * 5.0 - 96309.0", true));
    printf("Test 25 ===> %7.3f\n", evaluate("25.5 * 10.7 - 5.0 - 242.85", true));
    printf("Test 26 ===> %7.3f\n", evaluate("136.1 + 193.9 / 16.0 - 122.21875", true));
    printf("Test 27 ===> %7.3f\n", evaluate("177.2 + 119.3 - 18.0 - 251.5", true));
    printf("Test 28 ===> %7.3f\n", evaluate("2.8 / 11.9 + 17.0 + 10.7647058824", true));
    printf("Test 29 ===> %7.3f\n", evaluate("191.5 + 80.2 * 2.0 - 322.9", true));
    printf("Test 30 ===> %7.3f\n", evaluate("71.7 * 145.1 + 5.0 - 10378.67", true));
    // 4 operators 
    printf("Test 31 ===> %7.3f\n", evaluate("182.0 + 68.2 / 18.2 - 18.0 - 136.747252747", true));
    printf("Test 32 ===> %7.3f\n", evaluate("111.3 - 102.2 / 154.3 / 3.0 - 79.0792179736", true));
    printf("Test 33 ===> %7.3f\n", evaluate("174.1 - 176.7 + 106.0 - 10.0 - 60.4", true));
    printf("Test 34 ===> %7.3f\n", evaluate("138.1 / 52.5 - 150.4 + 15.0 + 166.76952381", true));
    printf("Test 35 ===> %7.3f\n", evaluate("108.7 - 31.6 - 195.9 / 7.0 - 14.1142857143", true));
    printf("Test 36 ===> %7.3f\n", evaluate("35.3 - 30.6 - 75.3 - 13.0 + 119.6", true));
    printf("Test 37 ===> %7.3f\n", evaluate("127.4 - 156.7 * 74.1 / 10.0 + 1070.747", true));
    printf("Test 38 ===> %7.3f\n", evaluate("137.2 - 66.3 + 63.1 - 8.0 - 88.0", true));
    printf("Test 39 ===> %7.3f\n", evaluate("139.8 - 44.0 + 190.3 + 5.0 - 252.1", true));
    printf("Test 40 ===> %7.3f\n", evaluate("102.5 / 162.3 * 2.6 * 5.0 + 31.7898952557", true));
    // 5 operators 
    printf("Test 41 ===> %7.3f\n", evaluate("3.5 / 146.3 + 103.2 - 6.7 / 6.0 - 61.1072567783", true));
    printf("Test 42 ===> %7.3f\n", evaluate("2.7 * 64.8 + 62.3 * 162.0 - 19.0 - 10206.56", true));
    printf("Test 43 ===> %7.3f\n", evaluate("187.4 + 81.8 - 150.7 / 100.1 - 1.0 - 223.694505495", true));
    printf("Test 44 ===> %7.3f\n", evaluate("41.2 + 154.7 * 24.5 * 25.3 / 10.0 - 9586.2795", true));
    printf("Test 45 ===> %7.3f\n", evaluate("37.2 / 100.3 * 8.3 / 115.2 + 20.0 + 24.9732780824", true));
    printf("Test 46 ===> %7.3f\n", evaluate("114.0 - 146.1 / 157.9 / 71.0 * 5.0 - 67.9348402002", true));
    printf("Test 47 ===> %7.3f\n", evaluate("97.1 + 99.6 / 177.9 * 28.4 - 18.0 - 48.0001686341", true));
    printf("Test 48 ===> %7.3f\n", evaluate("116.3 / 159.9 - 172.6 - 50.7 + 17.0 + 253.572670419", true));
    printf("Test 49 ===> %7.3f\n", evaluate("200.4 / 80.9 / 58.6 - 57.0 * 7.0 + 447.957728118", true));
    printf("Test 50 ===> %7.3f\n", evaluate("127.1 - 188.2 * 149.5 / 37.3 + 13.0 + 664.213672922", true));
    // 6 operators 
    printf("Test 51 ===> %7.3f\n", evaluate("62.3 + 186.3 * 3.4 * 39.7 / 93.2 + 20.0 - 301.115171674", true));
    printf("Test 52 ===> %7.3f\n", evaluate("176.5 - 148.2 + 190.9 - 100.8 * 142.7 - 16.0 + 14232.96", true));
    printf("Test 53 ===> %7.3f\n", evaluate("154.5 - 71.4 / 44.5 * 180.3 * 167.3 / 1.0 + 48296.7733933", true));
    printf("Test 54 ===> %7.3f\n", evaluate("165.5 - 191.8 - 6.0 / 25.7 + 113.5 * 5.0 - 486.966536965", true));
    printf("Test 55 ===> %7.3f\n", evaluate("176.0 / 192.4 * 44.6 + 100.7 * 163.2 / 5.0 - 3272.6463368", true));
    printf("Test 56 ===> %7.3f\n", evaluate("150.9 + 12.6 * 172.4 + 53.6 - 40.0 * 8.0 - 2000.74", true));
    printf("Test 57 ===> %7.3f\n", evaluate("34.9 - 185.8 + 15.7 - 125.8 / 95.2 + 12.0 + 181.521428571", true));
    printf("Test 58 ===> %7.3f\n", evaluate("137.3 / 57.8 * 85.5 / 134.0 * 50.3 + 12.0 - 30.2380887517", true));
    printf("Test 59 ===> %7.3f\n", evaluate("162.6 + 154.2 - 28.5 * 4.3 - 181.3 + 9.0 + 37.05", true));
    printf("Test 60 ===> %7.3f\n", evaluate("98.2 - 152.3 + 15.2 + 3.8 / 94.4 * 13.0 + 98.3766949153", true));
    // 7 operators 
    printf("Test 61 ===> %7.3f\n", evaluate("63.4 + 6.5 / 180.9 - 19.9 * 70.4 * 172.3 * 13.0 + 3138007.86807", true));
    printf("Test 62 ===> %7.3f\n", evaluate("61.3 + 165.4 + 156.1 + 63.0 * 112.3 + 63.2 - 20.0 - 7438.9", true));
    printf("Test 63 ===> %7.3f\n", evaluate("56.3 - 78.1 - 177.0 - 9.7 + 66.9 - 173.6 + 17.0 + 361.2", true));
    printf("Test 64 ===> %7.3f\n", evaluate("133.7 - 21.4 + 122.7 / 38.2 * 14.8 + 43.7 * 19.0 - 926.138219895", true));
    printf("Test 65 ===> %7.3f\n", evaluate("162.2 - 130.2 - 11.2 - 68.4 + 136.5 * 55.7 - 9.0 - 7481.45", true));
    printf("Test 66 ===> %7.3f\n", evaluate("140.3 - 144.9 * 127.7 * 178.3 + 137.1 - 52.4 / 13.0 + 3299007.68977", true));
    printf("Test 67 ===> %7.3f\n", evaluate("180.2 - 98.9 - 83.1 / 55.6 + 183.2 * 186.3 / 7.0 - 4888.54253854", true));
    printf("Test 68 ===> %7.3f\n", evaluate("3.3 * 77.1 * 33.9 + 25.3 - 10.2 - 199.7 - 13.0 - 8359.577", true));
    printf("Test 69 ===> %7.3f\n", evaluate("35.1 - 46.0 / 122.6 / 87.6 / 143.9 + 77.5 / 8.0 + 24.2125297648", true));
    printf("Test 70 ===> %7.3f\n", evaluate("196.0 / 38.4 * 188.6 + 101.6 + 23.8 + 38.1 - 15.0 - 1041.14583333", true));
    // 8 operators 
    printf("Test 71 ===> %7.3f\n", evaluate("117.3 - 116.1 - 36.6 + 55.1 + 129.3 + 82.7 + 44.2 / 19.0 - 163.026315789", true));
    printf("Test 72 ===> %7.3f\n", evaluate("199.2 - 5.6 + 21.4 * 167.1 * 151.2 - 53.3 * 181.4 + 4.0 - 531139.108", true));
    printf("Test 73 ===> %7.3f\n", evaluate("133.9 * 73.0 / 176.0 + 174.6 - 93.4 + 157.4 / 12.6 * 7.0 - 151.182512626", true));
    printf("Test 74 ===> %7.3f\n", evaluate("46.1 - 43.3 / 132.3 / 94.4 + 19.5 * 127.2 + 10.1 - 1.0 - 2461.59653298", true));
    printf("Test 75 ===> %7.3f\n", evaluate("36.0 * 1.3 - 35.8 - 9.5 / 33.5 * 139.8 + 172.7 + 15.0 - 84.0552238806", true));
    printf("Test 76 ===> %7.3f\n", evaluate("38.2 * 160.3 * 16.1 - 162.1 / 23.3 + 139.4 * 19.2 / 17.0 - 98662.1889185", true));
    printf("Test 77 ===> %7.3f\n", evaluate("19.3 + 18.0 + 54.7 / 167.1 + 176.0 / 1.1 - 198.5 - 11.0 + 88.8726511071", true));
    printf("Test 78 ===> %7.3f\n", evaluate("193.9 - 52.6 - 125.0 - 102.5 - 124.1 * 180.5 - 76.7 - 1.0 + 22641.95", true));
    printf("Test 79 ===> %7.3f\n", evaluate("192.2 - 153.1 * 78.5 + 52.9 - 83.6 * 40.0 + 40.9 - 6.0 + 15161.35", true));
    printf("Test 80 ===> %7.3f\n", evaluate("19.1 / 3.3 + 152.0 / 74.7 * 36.8 + 157.4 / 113.4 - 15.0 + 12.9432573971", true));
    return 0;
}


