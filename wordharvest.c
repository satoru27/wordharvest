#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

void find_exec(char*, char*, char*);
int valid_path(char *);
void clean_string(char *);
void process_words(char*, char*);
void clear_word_buffer(char*);
int search_word_in_file(char*, char*);
int char_is_valid(char);
int word_is_null(char*);
void write_word_on_output(char*, char*);

int main(int argc, char *argv[]){
	
	int  file_type_flag = 1;
	int  directory_flag = 1;
	int  output_flag = 1;

	int file_type_position = 0;
	int directory_position = 0;
	int output_position = 0;

	// Input handling
    for(int i = 1; i < argc; i++){
        if (strcmp(argv[i],"-e") == 0) {
        	printf("[*] File Type %s: %s \n", argv[i], argv[i+1]);
        	file_type_position = i;
        	file_type_flag--;	
        }
        if (strcmp(argv[i],"-d") == 0) {
        	printf("[*] Directory %s: %s \n", argv[i], argv[i+1]);
        	directory_position = i;
        	directory_flag--;
        }
        if (strcmp(argv[i],"-o") == 0) {
        	printf("[*] Output %s: %s \n", argv[i], argv[i+1]);
        	output_position = i;
        	output_flag--;
        }
    }

    printf("\n");

    if (directory_flag){
    	printf("[!] No search directory provided.\n");
    	printf("[!] Search directory must be provided with -d followed by a valid directory.\n");
    	printf("[!] Exiting...\n");
    	return 0;
    }

    if (output_flag){
    	printf("[!] No output file provided.\n");
    	printf("[!] Output file must be provided with -o followed by a valid filename (if file does not exist it will be created).\n");
    	printf("[!] Exiting...\n");
    	return 0;
    }

    char default_file_type[] = "txt:text";
    char * token;

    //Handle each file type as a token and uses the find command to search that file type on the given directory
    if (file_type_flag){
    	printf("[^] No file type provided, using default .txt and .text files type.\n");
    	printf("[^] File types can be provided with -e followed by the file types separated by \":\".\n\n");

    	char * token = strtok(default_file_type, ":");
    	while(token != NULL){
    		printf("[*] Executing command: ");
    		find_exec(token, argv[directory_position+1], argv[output_position+1]);
    		token = strtok(NULL, ":");
    	}
    	
    	printf("[*] Finished processing all file types\n");
    	printf("[*] Exiting...\n");
    }
    else{

    	char * token = strtok(argv[file_type_position+1], ":");
    	while(token != NULL){
    		printf("[*] Executing command: ");
    		find_exec(token, argv[directory_position+1], argv[output_position+1]);
    		token = strtok(NULL, ":");
    	}
    	
    	printf("\n[*] Finished processing all file types\n");
    	printf("[*] Exiting...\n");
    }



    return 0;
}

void find_exec(char* file_type, char* directory, char* output){
	/**
 	* Takes a file type and searches the given directory with the find command
 	*/
	char command[BUFFER_SIZE] = "find ";
	strcat(command, directory);
	strcat(command, " -type f -name \"*.");
	strcat(command, file_type);
	strcat(command, "\"");
	printf("%s\n", command);


	FILE *fp;
    char path[BUFFER_SIZE];

    
    fp = popen(command, "r");

    if (fp == NULL) {
      printf("[!] Failed to run command\n" );
      exit(1);
    }

    //Words on each found file are processed
    while (fgets(path, sizeof(path), fp) != NULL) {
      if (valid_path(path)){
 		clean_string(path);
      	printf("\n[*] Valid path: %s \n", path);
      	process_words(path, output);

      }

    }

    pclose(fp);
}

int valid_path(char* path){
	/**
 	* Simple check to see if a given path does not contain spaces
 	*/
	if(strstr(path, " ") == NULL){
		return 1;
	}
	else{
		return 0;
	}
}

void clean_string(char* path){
	/**
 	* Removes \n from a given string
 	*/
	char *pos;
	if((pos=strchr(path, '\n')) != NULL)
		*pos = '\0';
}

void process_words(char* path, char* output){
	/**
 	* Checks a given file for new words that can be appended to the output
 	*/
	char c[1];
	char newline[1] = "\n";
	char word_buffer[BUFFER_SIZE];
	clear_word_buffer(word_buffer);

	FILE *fp_in;
	fp_in = fopen(path,"r");

	if (fp_in == NULL){
		printf("[!] Error opening file: %s\n", path);
		return;
	}

	printf("[*] Processing words in %s and writing in %s ...\n", path, output);

	while( (c[0] = fgetc(fp_in)) != EOF ){
		if(char_is_valid(c[0])){
			// If a character is either a letter or number, it gets added to the buffer
			strncat(word_buffer, c, 1);
		
		}
		else{
			//If not, adds a newline to the buffer and checks if the word is already present in the output. 
			//If it is already at the output, the buffer is cleaned, if not, the word is added to the output.
			strncat(word_buffer, newline, 1);

			if(word_is_null(word_buffer) || search_word_in_file(word_buffer, output)){
			//If the word in the buffer is either a NULL Byte or an already added word, the buffer is cleaned.
				clear_word_buffer(word_buffer);
		
			}
			
			else{
			//If not, the word is added to the output and the buffer is cleaned
				write_word_on_output(word_buffer, output);
				clear_word_buffer(word_buffer);	
			
			}
		}
	}

	printf("\n");
	fclose(fp_in);
}

void clear_word_buffer(char* word_buffer){
	/**
 	* Sets the buffer to 0
 	*/
	memset(word_buffer, 0, BUFFER_SIZE);
}

int search_word_in_file(char* word, char* output){
	/**
 	* Checks if a given word present in the output
 	* If it is, returns 1 (True)
 	* If not, returns 0 (False)
 	*/
	FILE *fp;
	fp = fopen(output,"r");
	char word_buffer[BUFFER_SIZE];
	int flag = 0;

	if (fp == NULL){
		printf("[!] Error opening file: %s in search_word_in_file \n", output);
		return 1;
	}

	while (fgets(word_buffer, BUFFER_SIZE, fp) != NULL){
		//Reads the lines at output and compares it with the given word
		if (!(strcmp(word_buffer, word))){ 
			flag = 1;
			break;
		}
	}

	char temp[BUFFER_SIZE];
	if(flag){
		//Trailing \n is removed before the word is printed 
		strcpy(temp, word);
		clean_string(temp);
		printf("[^] Word \"%s\" already in %s \n", temp, output);
	}
	
	fclose(fp);
	return flag;
}

int word_is_null(char* word){
	/**
 	* Checks if a given word is a NULL byte
 	*/
	if(strlen(word) == 1){
		if (word[0] == 0){
			return 1;
		}
	}
	else{
		return 0;
	}

}

int char_is_valid(char c){
	/**
 	* Checks if the given character is between the following ASCII decimal ranges:
 	* 48 (0) to 57 (9)
 	* 65 (A) to 90 (Z)
 	* 97 (a) to 122 (z)
 	*/

	if (c >= 97 && c <= 122)
		return 1;
	
	if (c >= 48 && c <= 57)
		return 1;
	
	if (c >= 65 && c <= 90)
		return 1;

	else
		return 0;

}

void write_word_on_output(char* word, char* output){
	/**
 	* Writes the given word to the output
 	*/

	FILE *fp_out;
	fp_out = fopen(output,"a");
	
	if (fp_out == NULL){
		printf("[!] Error opening file: %s\n", output);
		return;
	}

	//Removes the trailing \n before the string is printed
	char temp[BUFFER_SIZE];
	strcpy(temp, word);
	clean_string(temp);
	printf("[*] Writing \"%s\" to %s \n", temp, output);
	
	fputs(word, fp_out);

	fclose(fp_out);
}
