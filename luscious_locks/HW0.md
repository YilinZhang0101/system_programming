# Welcome to Homework 0!

For these questions you'll need the mini course and  "Linux-In-TheBrowser" virtual machine (yes it really does run in a web page using javascript) at -

http://cs-education.github.io/sys/

Let's take a look at some C code (with apologies to a well known song)-
```C
// An array to hold the following bytes. "q" will hold the address of where those bytes are.
// The [] mean set aside some space and copy these bytes into teh array array
char q[] = "Do you wanna build a C99 program?";

// This will be fun if our code has the word 'or' in later...
#define or "go debugging with gdb?"

// sizeof is not the same as strlen. You need to know how to use these correctly, including why you probably want strlen+1

static unsigned int i = sizeof(or) != strlen(or);

// Reading backwards, ptr is a pointer to a character. (It holds the address of the first byte of that string constant)
char* ptr = "lathe"; 

// Print something out
size_t come = fprintf(stdout,"%s door", ptr+2);

// Challenge: Why is the value of away equal to 1?
int away = ! (int) * "";


// Some system programming - ask for some virtual memory

int* shared = mmap(NULL, sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
munmap(shared,sizeof(int*));

// Now clone our process and run other programs
if(!fork()) { execlp("man","man","-3","ftell", (char*)0); perror("failed"); }
if(!fork()) { execlp("make","make", "snowman", (char*)0); execlp("make","make", (char*)0)); }

// Let's get out of it?
exit(0);
```

## So you want to master System Programming? And get a better grade than B?
```C
int main(int argc, char** argv) {
	puts("Great! We have plenty of useful resources for you, but it's up to you to");
	puts(" be an active learner and learn how to solve problems and debug code.");
	puts("Bring your near-completed answers the problems below");
	puts(" to the first lab to show that you've been working on this.");
	printf("A few \"don't knows\" or \"unsure\" is fine for lab 1.\n"); 
	puts("Warning: you and your peers will work hard in this class.");
	puts("This is not CS225; you will be pushed much harder to");
	puts(" work things out on your own.");
	fprintf(stdout,"This homework is a stepping stone to all future assignments.\n");
	char p[] = "So, you will want to clear up any confusions or misconceptions.\n";
	write(1, p, strlen(p) );
	char buffer[1024];
	sprintf(buffer,"For grading purposes, this homework 0 will be graded as part of your lab %d work.\n", 1);
	write(1, buffer, strlen(buffer));
	printf("Press Return to continue\n");
	read(0, buffer, sizeof(buffer));
	return 0;
}
```
## Watch the videos and write up your answers to the following questions

**Important!**

The virtual machine-in-your-browser and the videos you need for HW0 are here:

http://cs-education.github.io/sys/

The coursebook:

http://cs341.cs.illinois.edu/coursebook/index.html

Questions? Comments? Use Ed: (you'll need to accept the sign up link I sent you)
https://edstem.org/

The in-browser virtual machine runs entirely in Javascript and is fastest in Chrome. Note the VM and any code you write is reset when you reload the page, *so copy your code to a separate document.* The post-video challenges (e.g. Haiku poem) are not part of homework 0 but you learn the most by doing (rather than just passively watching) - so we suggest you have some fun with each end-of-video challenge.

HW0 questions are below. Copy your answers into a text document (which the course staff will grade later) because you'll need to submit them later in the course. More information will be in the first lab.

## Chapter 1

In which our intrepid hero battles standard out, standard error, file descriptors and writing to files.

### Hello, World! (system call style)
1. Write a program that uses `write()` to print out "Hi! My name is `<Your Name>`".

#include <unistd.h>

int main() {
	write(1, "Hi! My name is Yilin Zhang\n", 27);
}

### Hello, Standard Error Stream!
2. Write a function to print out a triangle of height `n` to standard error.
   - Your function should have the signature `void write_triangle(int n)` and should use `write()`.
   - The triangle should look like this, for n = 3:
   ```C
   *
   **
   ***
   ```

#include <unistd.h>
void write_triangle(int n) {
	for (int i - 0;i < n; i ++) {
		for (int j =0; j < i; j ++) {
			write(STUERR_FILENO, "*", 1); 
		}
		write(STUERR_FILENO, "\n", 1); 
	}
}

int main() {
	scanf("%d", input);
	write_triangle(input);
}

### Writing to files
3. Take your program from "Hello, World!" modify it write to a file called `hello_world.txt`.
   - Make sure to to use correct flags and a correct mode for `open()` (`man 2 open` is your friend).

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
	mode_t mode = S_IRUSR | S_IWUSR;
	int fildes = open("hello_world.txt", O_CREAT | O_TRUNC |O_RDWR, mode);
	write(fildes, "Hi! My name is Yilin Zhang\n", 27);
	close(fildes);
}

### Not everything is a system call
4. Take your program from "Writing to files" and replace `write()` with `printf()`.
   - Make sure to print to the file instead of standard out!

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
	mode_t mode = S_IRUSR | S_IWUSR;
	int fildes = open("hello_world.txt", O_CREAT | O_TRUNC |O_RDWR, mode);
	if(fildes == -1) {
		perror ("open failed");
		exit(1);
	}
	printf("fildes is %d\n", fildes);
	close(fildes);
}

5. What are some differences between `write()` and `printf()`?

a. `printf()` internal buffering, which means data might not be written immediately to its destination until the buffer is full or  finishing a line, it will be printed out.
`write()` doesn't use any buffering and writes the data directly.

b. `write()` is a system call and is provided by the operating system.
`printf()` is a standard library function provided by the C Standard Library.

c. `write()` needs to include the header file <unistd.h>.
`printf()` needs to include the header file <stdio.h>.

## Chapter 2

Sizing up C types and their limits, `int` and `char` arrays, and incrementing pointers

### Not all bytes are 8 bits?
1. How many bits are there in a byte?
At least 8 bits.

2. How many bytes are there in a `char`?
1 byte

3. How many bytes the following are on your machine?
   - `int`, `double`, `float`, `long`, and `long long`

`int`: 4 bytes;
`double`: 8 bytes;
`float`: 4 bytes;
`long`: 4 bytes;
`long long`: 8 bytes;

### Follow the int pointer
4. On a machine with 8 byte integers:
```C
int main(){
    int data[8];
} 
```
If the address of data is `0x7fbd9d40`, then what is the address of `data+2`?

the address of `data+2` is `0x7fbd9d50`

5. What is `data[3]` equivalent to in C?
   - Hint: what does C convert `data[3]` to before dereferencing the address?

*(data+3) or 3[data]

### `sizeof` character arrays, incrementing pointers
  
Remember, the type of a string constant `"abc"` is an array.

6. Why does this segfault?
```C
char *ptr = "hello";
*ptr = 'J';
```
When declared a pointer and initialize it with a string literal, the pointer points to this read-only memory section.

7. What does `sizeof("Hello\0World")` return?
12

8. What does `strlen("Hello\0World")` return?
5

9. Give an example of X such that `sizeof(X)` is 3.
X = "Hi\0";

10. Give an example of Y such that `sizeof(Y)` might be 4 or 8 depending on the machine.

On a 32-bit system, pointers typically occupy 4 bytes (32 bits), so sizeof(void*) would return 4.

On a 64-bit system, pointers typically occupy 8 bytes (64 bits), so sizeof(void*) would return 8.


## Chapter 3

Program arguments, environment variables, and working with character arrays (strings)

### Program arguments, `argc`, `argv`
1. What are two ways to find the length of `argv`?

a. the length of argv is equivalent to the value of `argc`.
b. Iterating through argv until a NULL pointer is encountered:
	int count = 0;
	while (argv[count] != NULL) {
		count ++;
	}

2. What does `argv[0]` represent?

`argv[0]` represents the name of the program itself.

### Environment Variables
3. Where are the pointers to environment variables stored (on the stack, the heap, somewhere else)?

The pointers to the environment variables are typically stored in a section of the process's memory separate from the heap and the stack.
`char** environ` to access.

### String searching (strings are just char arrays)
4. On a machine where pointers are 8 bytes, and with the following code:
```C
char *ptr = "Hello";
char array[] = "Hello";
```
What are the values of `sizeof(ptr)` and `sizeof(array)`? Why?

`sizeof(ptr)`: 8. 
ptr is a pointer to a character. On a machine where pointers are 8 bytes, the size of any pointer would be 8 bytes.
`sizeof(array)`: 6.
array is an array of characters, and its size is the total number of bytes required to store the string, including the null terminator. 

### Lifetime of automatic variables

5. What data structure manages the lifetime of automatic variables?

## Chapter 4

Heap and stack memory, and working with structs

### Memory allocation using `malloc`, the heap, and time
1. If I want to use data after the lifetime of the function it was created in ends, where should I put it? How do I put it there?

You should allocate that data on the heap.

2. What are the differences between heap and stack memory?
 
heap: Memory is manually managed. Once allocated, it remains allocated until explicitly deallocated (freed) or the program ends.
`malloc()`, `calloc()`, `free()`

Stack: Memory is automatically managed. It's allocated when a function is called (for local variables) and deallocated when the function exits.

3. Are there other kinds of memory in a process?

Text Segment (or Code Segment), Data Segment(Initialized Data Segment/Uninitialized Data Segment (BSS)), Constant Data Segment.

4. Fill in the blank: "In a good C program, for every malloc, there is a ___free___".

### Heap allocation gotchas
5. What is one reason `malloc` can fail?

One primary reason `malloc` can fail is due to insufficient memory, and it will return a NULL pointer.

6. What are some differences between `time()` and `ctime()`?

`time()`: Returns the current calendar time (represented as the number of seconds since the Epoch, 1970-01-01 00:00:00 +0000, UTC). 

`ctime()`: Converts a given time in seconds (since the Epoch) into a human-readable string representation in the format "Wed Jun 30 21:49:08 1993\n".

7. What is wrong with this code snippet?
```C
free(ptr);
free(ptr);
```

free the same pointer `ptr` twice (double free).

8. What is wrong with this code snippet?
```C
free(ptr);
printf("%s\n", ptr);
```

After calling `free(ptr)`, the memory pointed to by `ptr` has been deallocated, and `ptr` is now a dangling pointer.

9. How can one avoid the previous two mistakes? 

Always ensure that a pointer is freed only once and preferably set the pointer to NULL after freeing to avoid such issues.

### `struct`, `typedef`s, and a linked list
10. Create a `struct` that represents a `Person`. Then make a `typedef`, so that `struct Person` can be replaced with a single word. A person should contain the following information: their name (a string), their age (an integer), and a list of their friends (stored as a pointer to an array of pointers to `Person`s).

#include <stdlib.h>
#include <string.h>

struct Person {
	string name;
	int age;
	struct Person* friends;
}

typedef struct Person Person_t;

11. Now, make two persons on the heap, "Agent Smith" and "Sonny Moore", who are 128 and 256 years old respectively and are friends with each other.

int main() {
	Person_t* ptr1 = (Person_t *) malloc(sizeof(Person_t));
	Person_t* ptr2 = (Person_t *) malloc(sizeof(Person_t));
	ptr1->name = "Agent Smith";
	ptr2->name = "Sonny Moore";
	ptr1->age = 128;
	ptr2->age = 256;
	ptr1->friend = ptr2;
	ptr2->friend = ptr1;

	free(ptr1);
	free(ptr2);
	return 0;
}

### Duplicating strings, memory allocation and deallocation of structures
Create functions to create and destroy a Person (Person's and their names should live on the heap).
12. `create()` should take a name and age. The name should be copied onto the heap. Use malloc to reserve sufficient memory for everyone having up to ten friends. Be sure initialize all fields (why?).

Person_t* create(string* cur_name, int* cur_age) {
	person_t* ret = (Person_t*) malloc(10 * sizeof(Person_t));
	ret->name = strdup(cur_name);
	ret->age = strdup(cur_age);

	return ret;
}

13. `destroy()` should free up not only the memory of the person struct, but also free all of its attributes that are stored on the heap. Destroying one person should not destroy any others.

void destroy(Person_t* p) {
	free(p->name);
	free(p->age);
	memset(p, 0, sizeof(Person_t));
	free(p);
}

## Chapter 5 

Text input and output and parsing using `getchar`, `gets`, and `getline`.

### Reading characters, trouble with gets
1. What functions can be used for getting characters from `stdin` and writing them to `stdout`?

getchar()
putchar()

2. Name one issue with `gets()`.

It can lead to buffer overflow vulnerabilities, which gets uncertain results.

### Introducing `sscanf` and friends
3. Write code that parses the string "Hello 5 World" and initializes 3 variables to "Hello", 5, and "World".

int main() {
	char* data = "Hello 5 World";
	char buffer1[10];
	int num;
	char buffer2[10];

	sscanf(data, "%s %d %s", buffer1, &num, buffer2);

	printf("String 1: %s\n", buffer1);
    printf("Number: %d\n", num);
    printf("String 2: %s\n", buffer2); 

	return 0;
}

### `getline` is useful
4. What does one need to define before including `getline()`?

#define _GUN_SOURCE

5. Write a C program to print out the content of a file line-by-line using `getline()`.

#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *file = fopen("*.txt", "r");
    if (!file) {
        perror("Failed to open file");
        return 1;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, file)) != -1) {
        printf("%s", line);
    }

    free(line);
    fclose(file);
    return 0;
}


## C Development

These are general tips for compiling and developing using a compiler and git. Some web searches will be useful here

1. What compiler flag is used to generate a debug build?

The `-g` flag is used to generate a debug build, which embeds debugging information into the executable. (gdb)

2. You modify the Makefile to generate debug builds and type `make` again. Explain why this is insufficient to generate a new build.

The `make` checks timestamps to determine which files to recompile. Without modify the source files, `make` may determine there's nothing to be recompiled.

We may should run `make clean` to remove old build artifacts before running make again.

3. Are tabs or spaces used to indent the commands after the rule in a Makefile?

Tabs.

4. What does `git commit` do? What's a `sha` in the context of git?

`git commit` creates a new commit object based on the changes staged for commit. This new commit object becomes a new point in the history of your repository.

A `sha` is a 40-character hexadecimal number that uniquely identifies a commit or object in the repository.

5. What does `git log` show you?

It shows the commit logs/history of the repository. (SHA, author, date, and the commit message of each commit in reverse chronological order.)

6. What does `git status` tell you and how would the contents of `.gitignore` change its output?

`git status` displays the state of the working directory and the staging area. It shows which changes are staged for the next commit, which are not, and which files are untracked.

The contents of `.gitignore` specify intentionally untracked files that Git should ignore. If a file matches a pattern listed in `.gitignore`, `git status` won't list it as an untracked file.

7. What does `git push` do? Why is it not just sufficient to commit with `git commit -m 'fixed all bugs' `?

It uploads the local repository commits to a remote repository.

Committing with `git commit -m 'fixed all bugs'` only saves a snapshot of the changes locally. Still need `push` to make these changes be reflected in a remote repository.

8. What does a non-fast-forward error `git push` reject mean? What is the most common way of dealing with this?

It means when trying to push your local changes to a remote repository, but there have been new commits on the remote that don't have been in the local branch.

The most common way is by first pulling the latest changes from the remote with `git pull` (resolving any merge conflicts), and then pushing the changes with `git push`. 
Use `git pull --rebase` to reapply local changes on top of the remote changes before pushing.


## Optional (Just for fun)
- Convert your a song lyrics into System Programming and C code and share on Ed.
- Find, in your opinion, the best and worst C code on the web and post the link to Ed.
- Write a short C program with a deliberate subtle C bug and post it on Ed to see if others can spot your bug.
- Do you have any cool/disastrous system programming bugs you've heard about? Feel free to share with your peers and the course staff on Ed.
