#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <windows.h>

struct Date {
	int day;
	int month;
	int year;
};

typedef enum { started, DNFed, finished, addedToTbr } Action;
typedef enum { nonFiction, contemporary, mystery, horror, historicalFiction, romance, fantasy, sciFi, classic } Genre;

struct User {
	int id;
	char username[30];
	char country[30];
	int booksRead;
	int tbr;
	int dnf;
} users[100];

struct Book {
	char title[50];
	char author[50];
	int pageNum;
	Genre genre;
	int shelved;
	float avgRating;
	struct Date pubDate;
} books[100];

struct communityPost {
	int id;
	struct User user;
	struct Book book;
	Action action;
	struct Date date;
	int rating;
} communityPosts[100];

int currentUserID;

void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void clearScreen() {
	system("cls");
}

const char * stringifyActionEnum(int value) {
	static char* actions[] = { "started", "did not finish", "finished", "added to TBR" };
	return actions[value];
}

const char * stringifyGenreEnum(int value) {
	static char* genres[] = { "non-fiction", "contemporary", "mystery", "horror", "historical fiction", "romance", "fantasy", "sci-fi", "classic" };
	return genres[value];
}

float getAvgRating(char * title, char * author) {
	float counter = 0;
	float allRatings = 0;
	int numPosts = 0;
 	loadPostsFromFile(communityPosts, &numPosts);

	for(int i = 0; i < sizeof(communityPosts)/sizeof(struct communityPost); i++) {
		if(strcmp(communityPosts[i].book.title, title) == 0 && strcmp(communityPosts[i].book.author, author) == 0 && communityPosts[i].rating) {
			counter++;
			allRatings += communityPosts[i].rating;
		}
	}

	if (counter != 0) {
        return (float)allRatings / counter;
    } else {
        return 0.00;
    }
}

int doesBookExist(char * title, char * author) {
	int numBooks = 0;
	loadBooksFromFile(books, &numBooks);

	for(int i = 0; i < numBooks; i++) {
		if(strcmp(title, books[i].title) == 0 && strcmp(author, books[i].author) == 0) {
			return i;
		}
	}

	return -1;
}

void next(int id) {
	setColor(7); // default
	printf("\nPress any key to continue...");
    getch(); // Wait for user input before continuing
    userPrompts(currentUserID);
}

void truncateString(char * input, char * output, int maxLen) {
    int inputLen = strlen(input);

    if (inputLen >= maxLen) {
        strncpy(output, input, maxLen - 7); // -3 to account for ellipsis
        output[maxLen - 7] = '\0'; // Null-terminate the truncated string
        strcat(output, "..."); // Append ellipsis
    } else {
        strcpy(output, input); // Use the original string
    }
}

void addBookAvgRating() {
	int numBooks = 0;
	loadBooksFromFile(books, &numBooks);

	for(int i = 0; i < numBooks; i++) {
		books[i].avgRating = getAvgRating(books[i].title, books[i].author);
	}

	saveBooksToFile(books, numBooks);
}

int genreSelection() {
	char ch;
	int g;
	printf("\n\033[36m[1]\033[0m Non-fiction.\n");
    printf("\033[36m[2]\033[0m Contemporary.\n");
    printf("\033[36m[3]\033[0m Mystery.\n");
    printf("\033[36m[4]\033[0m Horror.\n");
    printf("\033[36m[5]\033[0m Historical fiction.\n");
    printf("\033[36m[6]\033[0m Romance.\n");
    printf("\033[36m[7]\033[0m Fantasy.\n");
    printf("\033[36m[8]\033[0m Science fiction.\n");
    printf("\033[36m[9]\033[0m Classic.\n");

    setColor(11); //cyan
    printf("\n> ");
    setColor(7); //default
    ch = getch();
    g = ch - '0';

    return g;
}

int actionSelection(struct communityPost *post, int userId) {
    char ch;
    int a;
    int numUsers = 0;
    loadUsersFromFile(users, &numUsers);

	printf("\n[\033[36m1\033[0m] You started reading \033[36m%s\033[0m.\n", post->book.title);
    printf("[\033[36m2\033[0m] You gave up on \033[36m%s\033[0m.\n", post->book.title);
    printf("[\033[36m3\033[0m] You finished reading \033[36m%s\033[0m.\n", post->book.title);
    printf("[\033[36m4\033[0m] You want to read \033[36m%s\033[0m.\n", post->book.title);

    setColor(11); // cyan
    printf("\n> ");
    setColor(7); // default
    ch = getch();
    a = ch - '0';

    switch(a) {
        case 1:
            post->action = started;
            break;
        case 2:
            post->action = DNFed;
            post->user.dnf++;
            users[userId].dnf++;
			break;
        case 3:
            post->action = finished;
            post->user.booksRead++;
            users[userId].booksRead++;
			break;
        case 4:
            post->action = addedToTbr;
            post->user.tbr++;
            users[userId].tbr++;
            break;
        default:
            setColor(12); // red
            printf("\nInvalid command prompt. Try again.\n> ");
            setColor(7); // default
            getch();
            break;
    }

    saveUsersToFile(users, numUsers); // Update the user's file

    return a;
}

void saveCommunityPostsToFile(struct communityPost posts[], int numPosts) {
    FILE *file = fopen("community_posts.txt", "w");
    if (file == NULL) {
    	setColor(12);
        printf("Error opening file for writing.\n");
        setColor(7);
        return;
    }

    for (int i = 0; i < numPosts; i++) {
        fprintf(file, "%d,%d,%s,%s,%s,%d/%d/%d,%d,%d\n",
            posts[i].id,
            posts[i].user.id,
            posts[i].user.username,
            posts[i].book.title,
            posts[i].book.author,
            posts[i].date.day,
            posts[i].date.month,
            posts[i].date.year,
            (int)posts[i].action,
            posts[i].rating);
    }

    fclose(file);
}

void loadPostsFromFile(struct communityPost posts[], int *numPosts) {
    FILE *file = fopen("community_posts.txt", "r");
    if (file == NULL) {
    	setColor(12);
        printf("Error opening file for reading.\n");
        setColor(7);
        return;
    }

    *numPosts = 0;
    while (fscanf(file, "%d,%d,%[^,],%[^,],%[^,],%d/%d/%d,%d,%d\n", &posts[*numPosts].id, &posts[*numPosts].user.id, posts[*numPosts].user.username,
				 posts[*numPosts].book.title, posts[*numPosts].book.author, &posts[*numPosts].date.day, &posts[*numPosts].date.month,
				 &posts[*numPosts].date.year, &posts[*numPosts].action, &posts[*numPosts].rating) != EOF) {
        (*numPosts)++;
    }

    fclose(file);
}

void saveBooksToFile(struct Book books[], int numBooks) {
    FILE *file = fopen("books.txt", "w");
    if (file == NULL) {
        printf("Error opening file for writing.\n");
        return;
    }

    for (int i = 0; i < numBooks; i++) {
        fprintf(file, "%s,%s,%d,%d/%d/%d,%d,%.2f,%d\n",
			books[i].title, books[i].author, (int)books[i].genre, books[i].pubDate.day, books[i].pubDate.month, books[i].pubDate.year,
			books[i].pageNum, books[i].avgRating, books[i].shelved);
    }

    fclose(file);
}

void loadBooksFromFile(struct Book books[], int *numBooks) {
    FILE *file = fopen("books.txt", "r");
    if (file == NULL) {
        printf("Error opening file for reading.\n");
        return;
    }

    *numBooks = 0;
    while (fscanf(file, "%[^,],%[^,],%d,%d/%d/%d,%d,%.2f,%d\n",
				  books[*numBooks].title, books[*numBooks].author, &books[*numBooks].genre,
                  &books[*numBooks].pubDate.day, &books[*numBooks].pubDate.month, &books[*numBooks].pubDate.year,
				  &books[*numBooks].pageNum, &books[*numBooks].avgRating, &books[*numBooks].shelved) != EOF) {
        (*numBooks)++;
    }

    fclose(file);
}


void addEntry(int id) {
	time_t current_time;
    struct tm* time_info;
	int day, month, year;
	int x, y, g;
	int rating;
	int numPosts = 0;
	int numBooks = 0;

	loadPostsFromFile(communityPosts, &numPosts);
	loadBooksFromFile(books, &numBooks);

	for(int i = 0; i < sizeof(communityPosts)/sizeof(struct communityPost); i++) {
		if (strcmp(communityPosts[i].user.username, "") == 0) {
			communityPosts[i].id = i;
			strcpy(communityPosts[i].user.username, users[id].username);
			communityPosts[i].user.id = currentUserID;
			strcpy(communityPosts[i].user.country, users[id].country);

			// Get current date and assign it to the post's date
			time(&current_time);
    		time_info = localtime(&current_time);

    		communityPosts[i].date.day = time_info->tm_mday;
    		communityPosts[i].date.month = time_info->tm_mon + 1;  // tm_mon is 0-indexed, so add 1
    		communityPosts[i].date.year = time_info->tm_year + 1900;  // tm_year is years since 1900

			setColor(14); //yellow
			printf("Insert book information:\n");
			setColor(3); // Blue
			printf("Title\t: ");
			setColor(7); //default
			while (getchar() != '\n');
			do {
    			fgets(communityPosts[i].book.title, sizeof(communityPosts[i].book.title), stdin);
    			communityPosts[i].book.title[strcspn(communityPosts[i].book.title, "\n")] = '\0'; // remove trailing newline

    			if (strlen(communityPosts[i].book.title) == 0) {
        			setColor(12); //red
        			printf("Title cannot be empty. Try again.\nTitle\t: ");
        			setColor(7); //default
    			}
			} while (strlen(communityPosts[i].book.title) == 0);

			setColor(3); // Blue
			printf("Author\t: ");
			setColor(7); // default
			do {
    			fgets(communityPosts[i].book.author, sizeof(communityPosts[i].book.author), stdin);
    			communityPosts[i].book.author[strcspn(communityPosts[i].book.author, "\n")] = '\0'; // remove trailing newline

    			if (strlen(communityPosts[i].book.author) == 0) {
        			setColor(12); //red
        			printf("Author cannot be empty. Try again.\nAuthor\t: ");
        			setColor(7); //default
    			}
			} while (strlen(communityPosts[i].book.author) == 0);

			if(doesBookExist(communityPosts[i].book.title, communityPosts[i].book.author) != -1) {
				books[doesBookExist(communityPosts[i].book.title, communityPosts[i].book.author)].shelved++;
				communityPosts[i].book = books[doesBookExist(communityPosts[i].book.title, communityPosts[i].book.author)];
				saveBooksToFile(books, numBooks);
			} else {
   				g = genreSelection();

    			switch(g) {
					case 1:
						communityPosts[i].book.genre = nonFiction;
						break;
					case 2:
						communityPosts[i].book.genre = contemporary;
						break;
					case 3:
						communityPosts[i].book.genre = mystery;
						break;
					case 4:
						communityPosts[i].book.genre = horror;
						break;
					case 5:
						communityPosts[i].book.genre = historicalFiction;
						break;
					case 6:
						communityPosts[i].book.genre = romance;
						break;
					case 7:
						communityPosts[i].book.genre = fantasy;
						break;
					case 8:
						communityPosts[i].book.genre = sciFi;
						break;
					case 9:
						communityPosts[i].book.genre = classic;
						break;
					default:
						setColor(12); //red
						printf("\nInvalid command prompt. Try again.\n> ");
						setColor(7); //default
						getch();
						break;
				}

				setColor(11); //cyan
				printf("You selected %s", stringifyGenreEnum(communityPosts[i].book.genre));
				setColor(3); // Blue
				printf("\nNumber of pages\t: ");
				setColor(7); // default
				scanf("%d", &communityPosts[i].book.pageNum);
				setColor(3); // Blue
				printf("Publication date (DD/MM/YYYY): ");
				setColor(7); // default
				scanf("%d/%d/%d", &day, &month, &year);
				while (getchar() != '\n');
				// Check if the date is valid
				while(!isDateValid(day, month, year)) {
					setColor(12); //red
					printf("Invalid date format. Try again.\n> ");
					setColor(7); //default
					scanf("%d/%d/%d", &day, &month, &year);
				}

				communityPosts[i].book.pubDate.day = day;
				communityPosts[i].book.pubDate.month = month;
				communityPosts[i].book.pubDate.year = year;

				for(int k = 0; k <= numBooks; k++) {
					if(strcmp(books[k].title, "") == 0) {
						books[k] = communityPosts[i].book;
						books[k].shelved = 1;
						break;
					}
				}
				numBooks++;
    			saveBooksToFile(books, numBooks);
			}

			x = actionSelection(&communityPosts[i], id);

			setColor(11); //cyan
			switch(x) {
				case 1:
				case 2:
				case 4:
					printf("You %s %s by %s\n", stringifyActionEnum(communityPosts[i].action), communityPosts[i].book.title, communityPosts[i].book.author);
					break;
				case 3:
					printf("You %s %s by %s\n", stringifyActionEnum(communityPosts[i].action), communityPosts[i].book.title, communityPosts[i].book.author);

					setColor(3); // Blue
					printf("\nWhat would you like to rate this book? (1 to 5)");
					setColor(11); //cyan
					printf("\n> ");
					setColor(7); //default
					scanf("%d", &rating);
					while(rating != 1 && rating != 2 && rating != 3 && rating != 4 && rating != 5) {
						setColor(12); //red
						printf("You can only insert values from 1 to 5. Try again.\n> ");
						setColor(7); //default
						scanf("%d", &rating);
					}
					communityPosts[i].rating = rating;
					break;
				default:
					setColor(12); //red
					printf("\nInvalid command prompt. Try again.\n> ");
					setColor(7); //default
					getch();
					break;
			}

			// Get average rating
			communityPosts[i].book.avgRating = getAvgRating(communityPosts[i].book.author, communityPosts[i].book.title);
			addBookAvgRating();

			numPosts++;
        	saveCommunityPostsToFile(communityPosts, numPosts);

			setColor(11); // cyan
			printf("New entry added successfully!\n");
			next(currentUserID);
			// exit loop
			break;
		}
	}
}

void displayAllPosts(id) {
	int i = 0;
	int startX = 2;
    int startY = 2;

    int maxColWidth = 27;

    int numPosts;
	loadPostsFromFile(communityPosts, &numPosts);

    // Print table headers
    gotoxy(startX, startY);
    for(int j = 0; j <= maxColWidth * 4 ; j++) {
    	if(j == 0) {
    		printf("%c", 201);
		} else if (j == maxColWidth || j == maxColWidth * 2 || j == maxColWidth * 3) {
			printf("%c", 203);
		} else if(j == maxColWidth * 4) {
			printf("%c", 187);
		} else {
			printf("%c", 205);
		}
	}

    gotoxy(startX, startY + 1);
	printf("%c", 186);
	setColor(11); // cyan
	printf("%*sUser%*s", (maxColWidth - 4) / 2, " ", (maxColWidth - 5) / 2, " ");
	setColor(7); // default
	printf("%c", 186);
	setColor(11); // cyan
	printf("%*sActivity%*s", (maxColWidth - 8) / 2, " ", (maxColWidth - 9) / 2, " ");
	setColor(7); // default
	printf("%c", 186);
	setColor(11); // cyan
	printf("%*sBook%*s", (maxColWidth - 4) / 2, " ", (maxColWidth - 5) / 2, " ");
	setColor(7); // default
	printf("%c", 186);
	setColor(11); // cyan
	printf("%*sRating%*s", (maxColWidth - 6) / 2, " ", (maxColWidth - 6) / 2, " ");
	setColor(7); // default
	printf("%c", 186);
    gotoxy(startX, startY + 2);
    for(int j = 0; j <= maxColWidth * 4; j++) {
    	if(j == 0) {
			printf("%c", 204);
		} else if(j == maxColWidth || j == maxColWidth * 2 || j == maxColWidth * 3) {
			printf("%c", 206);
		} else if(j == maxColWidth * 4) {
			printf("%c", 185);
		} else {
			printf("%c", 205);
		}
	}
	// Print table rows
	while(strcmp(communityPosts[i].book.title, "") != 0) {
		gotoxy(startX, startY + i + 3);
        printf("%c", 186);

        // Print User
        char username[maxColWidth + 1];
        truncateString(communityPosts[i].user.username, username, maxColWidth);
        int padding = maxColWidth - strlen(username);
        int spacesBefore = padding / 2;
        int spacesAfter = padding - spacesBefore;
		printf("%*s%s%*s%c", spacesBefore - 1, " ", username, spacesAfter, " ", 186);

        // Print Activity
        char action[maxColWidth + 1];
        truncateString(stringifyActionEnum(communityPosts[i].action), action, maxColWidth);
        padding = maxColWidth - strlen(action);
        spacesBefore = padding / 2;
        spacesAfter = padding - spacesBefore;
        printf("%*s%s%*s%c", spacesBefore - 1, " ", action, spacesAfter, " ", 186);

        // Print Book
        char title[maxColWidth + 1];
        truncateString(communityPosts[i].book.title, title, maxColWidth);
        padding = maxColWidth - strlen(title);
        spacesBefore = padding / 2;
        spacesAfter = padding - spacesBefore;

        printf("%*s%s%*s%c", spacesBefore - 1, " ", title, spacesAfter, " ", 186);

        // Print Rating
        padding = maxColWidth - 3;
        spacesBefore = padding / 2;
        spacesAfter = padding - spacesBefore;
        if (communityPosts[i].rating > 0) {
        	if(communityPosts[i].rating > 3) {
        		setColor(11); // cyan
			} else if(communityPosts[i].rating < 3) {
				setColor(12); // red
			} else {
				setColor(14); // yellow
			}
            printf("%*s%d/5%*s", spacesBefore - 1, " ", communityPosts[i].rating, spacesAfter, " ");
            setColor(7); //default
            printf("%c", 186);
        } else {
            printf("%*sN/A%*s%c", spacesBefore - 1, " ", spacesAfter, " ", 186);
        }

		i++;
	}

	// Print table bottom line
    gotoxy(startX, startY + i + 3);
    for(int j = 0; j <= maxColWidth * 4; j++) {
    	if(j == 0) {
    		printf("%c", 200);
		} else if(j == maxColWidth || j == maxColWidth * 2 || j == maxColWidth * 3) {
			printf("%c", 202);
		} else if(j == maxColWidth * 4) {
			printf("%c", 188);
		} else {
			printf("%c", 205);
		}
	}

    // Print prompt to continue
    gotoxy(startX, startY + i + 5);
    next(currentUserID);
}

void displayAllUsers(int id) {
	int i = 0;
	int startX = 5;
	int startY = 2;

	// print header
	setColor(11); // cyan
	gotoxy(startX, startY);
	printf("Username");
	gotoxy(startX + 20, startY);
	printf("Country");
	gotoxy(startX + 40, startY);
	printf("Books Read");
	gotoxy(startX + 60, startY);
	printf("DNF(did not finish)");
	gotoxy(startX + 80, startY);
	printf("TBR(to be read)");
	gotoxy(startX - 1, startY + 1);
	for(int k = 0; k <= startX + 84; k++) {
		printf("-");
	}

	int numUsers;
	loadUsersFromFile(users, &numUsers);

	setColor(7); // default
	while(strcmp(users[i].username, "") != 0) {
		gotoxy(startX, startY + i + 2);
		printf("%s", users[i].username);
		gotoxy(startX + 20, startY + i + 2);
		printf("%s", users[i].country);
		gotoxy(startX + 40, startY + i + 2);
		printf("%d", users[i].booksRead);
		gotoxy(startX + 60, startY + i + 2);
		printf("%d", users[i].dnf);
		gotoxy(startX + 80, startY + i + 2);
		printf("%d", users[i].tbr);

		i++;
	}

	next(currentUserID);
}

void displayAllBooks(id) {
	int i = 0;
	int startX = 2;
    int startY = 2;
    int numBooks = 0;

    int maxColWidth = 18;

    loadBooksFromFile(books, &numBooks);

    // Print table headers
    gotoxy(startX, startY);
    for(int j = 0; j <= maxColWidth * 6 ; j++) {
    	if(j == 0) {
    		printf("%c", 201);
		} else if (j == maxColWidth || j == maxColWidth * 2 || j == maxColWidth * 3 || j == maxColWidth * 4 || j == maxColWidth * 5) {
			printf("%c", 203);
		} else if(j == maxColWidth * 6) {
			printf("%c", 187);
		} else {
			printf("%c", 205);
		}
	}

    gotoxy(startX, startY + 1);
	printf("%c", 186);
	setColor(11); // cyan
	printf("%*sTitle%*s", (maxColWidth - 5) / 2, " ", (maxColWidth - 6) / 2, " ");
	setColor(7); // default
	printf("%c", 186);
	setColor(11); // cyan
	printf("%*sAuthor%*s", (maxColWidth - 6) / 2, " ", (maxColWidth - 7) / 2, " ");
	setColor(7); // default
	printf("%c", 186);
	setColor(11); // cyan
	printf("%*sGenre%*s", (maxColWidth - 5) / 2, " ", (maxColWidth - 6) / 2, " ");
	setColor(7); // default
	printf("%c", 186);
	setColor(11); // cyan
	printf("%*sPages%*s", (maxColWidth - 5) / 2, " ", (maxColWidth - 6) / 2, " ");
	setColor(7); // default
	printf("%c", 186);
	setColor(11); // cyan
	printf("%*sAvg Rating%*s", (maxColWidth - 10) / 2, " ", (maxColWidth - 11) / 2, " ");
	setColor(7); // default
	printf("%c", 186);
	setColor(11); // cyan
	printf("%*sPub date%*s", (maxColWidth - 8) / 2, " ", (maxColWidth - 9) / 2, " ");
	setColor(7); // default
	printf("%c", 186);
    gotoxy(startX, startY + 2);
    for(int j = 0; j <= maxColWidth * 6; j++) {
    	if(j == 0) {
			printf("%c", 204);
		} else if(j == maxColWidth || j == maxColWidth * 2 || j == maxColWidth * 3 || j == maxColWidth * 4 || j == maxColWidth * 5) {
			printf("%c", 206);
		} else if(j == maxColWidth * 6) {
			printf("%c", 185);
		} else {
			printf("%c", 205);
		}
	}

	// Print table rows
	while(strcmp(books[i].title, "") != 0) {
		gotoxy(startX, startY + i + 3);
        printf("%c", 186);

        // Print Title
        char title[maxColWidth + 1]; // +1 for null terminator
        truncateString(books[i].title, title, maxColWidth);
        int padding = maxColWidth - strlen(title);
        int spacesBefore = padding / 2;
        int spacesAfter = padding - spacesBefore;
		printf("%*s%s%*s%c", spacesBefore - 1, " ", title, spacesAfter, " ", 186);

		// Print Author
		char author[maxColWidth + 1]; // +1 for null terminator
        truncateString(books[i].author, author, maxColWidth);
        padding = maxColWidth - strlen(author);
        spacesBefore = padding / 2;
        spacesAfter = padding - spacesBefore;
		printf("%*s%s%*s%c", spacesBefore - 1, " ", author, spacesAfter, " ", 186);

        // Print Genre
        char genre[maxColWidth + 1];
        truncateString(stringifyGenreEnum(books[i].genre), genre, maxColWidth);
        padding = maxColWidth - strlen(genre);
        spacesBefore = padding / 2;
        spacesAfter = padding - spacesBefore;
        printf("%*s%s%*s%c", spacesBefore - 1, " ", genre, spacesAfter, " ", 186);

        // Print Pages
        int pagesSize = 0;
        if(books[i].pageNum < 10) {
			pagesSize = 1;
		} else if(books[i].pageNum < 100) {
			pagesSize = 2;
		} else if(books[i].pageNum < 1000) {
			pagesSize = 3;
		} else {
			pagesSize = 4;
		}
        padding = maxColWidth - pagesSize;
        spacesBefore = padding / 2;
        spacesAfter = padding - spacesBefore;
		printf("%*s%d%*s%c", spacesBefore - 1, " ", books[i].pageNum, spacesAfter, " ", 186);

        // Print Average rating
        books[i].avgRating = getAvgRating(books[i].title, books[i].author);
        padding = maxColWidth - 4; // 4 because our output will look like 4.50 for example
        spacesBefore = padding / 2;
        spacesAfter = padding - spacesBefore;
        if (books[i].avgRating == 0.00) {
    		printf("%*sN/A %*s", spacesBefore - 1, " ", spacesAfter, " ");
		} else if (books[i].avgRating > 3) {
    		setColor(11); // cyan
    		printf("%*s%.2f%*s", spacesBefore - 1, " ", books[i].avgRating, spacesAfter, " ");
		} else if (books[i].avgRating < 3) {
    		setColor(12); // red
    		printf("%*s%.2f%*s", spacesBefore - 1, " ", books[i].avgRating, spacesAfter, " ");
		} else {
    		setColor(14); // yellow
    		printf("%*s%.2f%*s", spacesBefore - 1, " ", books[i].avgRating, spacesAfter, " ");
		}
		setColor(7); // default
		printf("%c", 186);

		// Print Publication Date
		int dayLen = books[i].pubDate.day < 10 ? 1 : 2;
		int monthLen = books[i].pubDate.month < 10 ? 1 : 2;
        padding = maxColWidth - (dayLen + monthLen + 6);
        spacesBefore = padding / 2;
        spacesAfter = padding - spacesBefore;
		printf("%*s%d/%d/%d%*s%c", spacesBefore - 1, " ", books[i].pubDate.day, books[i].pubDate.month, books[i].pubDate.year, spacesAfter, " ", 186);

		i++;
	}

	// Print table bottom line
    gotoxy(startX, startY + i + 3);
    for(int j = 0; j <= maxColWidth * 6; j++) {
    	if(j == 0) {
    		printf("%c", 200);
		} else if(j == maxColWidth || j == maxColWidth * 2 || j == maxColWidth * 3 || j == maxColWidth * 4 || j == maxColWidth * 5) {
			printf("%c", 202);
		} else if(j == maxColWidth * 6) {
			printf("%c", 188);
		} else {
			printf("%c", 205);
		}
	}

    // Print prompt to continue
    gotoxy(startX, startY + i + 5);
    next(currentUserID);
}

void displayBooksByGenre(id) {
	int genre;
	int booksFound = 0;
	int numBooks = 0;

	genre = genreSelection();

	loadBooksFromFile(books, &numBooks);

	for(int i = 0; i < sizeof(books)/sizeof(struct Book); i++) {
		if(books[i].genre == genre - 1 && strcmp(books[i].title, "") != 0) {
			booksFound = 1;
			printf("Title: %s\n", books[i].title);
            printf("Author: %s\n", books[i].author);
            printf("Number of Pages: %d\n", books[i].pageNum);
            if(books[i].avgRating > 3) {
        		setColor(11); // cyan
			} else if(books[i].avgRating < 3) {
				setColor(12); // red
			} else {
				setColor(14); // yellow
			}
            printf("Average Rating: %.2f\n", books[i].avgRating);
            setColor(7); //default
            printf("Publication Date: %d/%d/%d\n", books[i].pubDate.day, books[i].pubDate.month, books[i].pubDate.year);
            setColor(11); //cyan
            printf("--------------------------------\n\n");
            setColor(7); //default
		}
	}

	if(!booksFound) {
		setColor(12); //red
        printf("No books found in this genre.");
        setColor(7); // default
	}
	next(currentUserID);
}

void highestAndLowestRatedBook(userId) {
	int startX = 2;
	int startY = 2;
	int numBooks = 0;

	loadBooksFromFile(books, &numBooks);

	struct Book highestRated = books[0];
	struct Book lowestRated = books[0];
	for(int i = 0; i < sizeof(books)/sizeof(struct Book); i++) {
		if(books[i].avgRating > highestRated.avgRating) {
			highestRated = books[i];
		}

		if(books[i].avgRating < lowestRated.avgRating && books[i].avgRating != 0.00) {
			lowestRated = books[i];
		}
	}
	// display highest rated book
	gotoxy(startX, startY);
	setColor(11); // cyan
	printf("Highest rated book:");
	gotoxy(startX, startY + 1);
	for(int k = 0; k < 30; k++) {
		printf("-");
	}
	gotoxy(startX, startY + 2);
	printf("Title:");
	gotoxy(startX + 20, startY + 2);
	setColor(7); // default
	printf("%s", highestRated.title);
	gotoxy(startX, startY + 3);
	setColor(11); // cyan
	printf("Author:");
	gotoxy(startX + 20, startY + 3);
	setColor(7); // default
	printf("%s", highestRated.author);
	gotoxy(startX, startY + 4);
	setColor(11); // cyan
	printf("Genre:");
	gotoxy(startX + 20, startY + 4);
	setColor(7); // default
	printf("%s", stringifyGenreEnum(highestRated.genre));
	gotoxy(startX, startY + 5);
	setColor(11); // cyan
	printf("Average Rating:");
	gotoxy(startX + 20, startY + 5);
	printf("%.2f", highestRated.avgRating);
	gotoxy(startX, startY + 6);
	printf("Publication Date:");
	gotoxy(startX + 20, startY + 6);
	setColor(7); // default
	printf("%d/%d/%d", highestRated.pubDate.day, highestRated.pubDate.month, highestRated.pubDate.year);


	// display lowest rated book
	gotoxy(startX + 50, startY);
	setColor(12); // red
	printf("Lowest rated book:");
	gotoxy(startX + 50, startY + 1);
	for(int k = 0; k < 30; k++) {
		printf("-");
	}
	gotoxy(startX + 50, startY + 2);
	printf("Title:");
	gotoxy(startX + 70, startY + 2);
	setColor(7); // default
	printf("%s", lowestRated.title);
	gotoxy(startX + 50, startY + 3);
	setColor(12); // red
	printf("Author:");
	gotoxy(startX + 70, startY + 3);
	setColor(7); // default
	printf("%s", lowestRated.author);
	gotoxy(startX + 50, startY + 4);
	setColor(12); // red
	printf("Genre:");
	gotoxy(startX + 70, startY + 4);
	setColor(7); // default
	printf("%s", stringifyGenreEnum(lowestRated.genre));
	gotoxy(startX + 50, startY + 5);
	setColor(12); // red
	printf("Average Rating:");
	gotoxy(startX + 70, startY + 5);
	printf("%.2f", lowestRated.avgRating);
	gotoxy(startX + 50, startY + 6);
	printf("Publication Date:");
	gotoxy(startX + 70, startY + 6);
	setColor(7); // default
	printf("%d/%d/%d", lowestRated.pubDate.day, lowestRated.pubDate.month, lowestRated.pubDate.year);


	gotoxy(startX, startY + 8);
	next(currentUserID);
}

void mostAndLeastPopular(userId) {
	int startX = 2;
	int startY = 2;
	int numBooks = 0;
	loadBooksFromFile(books, &numBooks);

	struct Book mostPopular = books[0];
	struct Book leastPopular = books[0];
	for(int i = 0; i < sizeof(books)/sizeof(struct Book); i++) {
		if(books[i].shelved > mostPopular.shelved) {
			mostPopular = books[i];
		}

		if(books[i].shelved < leastPopular.shelved && books[i].shelved > 0) {
			leastPopular = books[i];
		}
	}
	// display most popular book
	gotoxy(startX, startY);
	setColor(11); // cyan
	printf("Most popular book:");
	gotoxy(startX, startY + 1);
	for(int k = 0; k < 30; k++) {
		printf("-");
	}
	gotoxy(startX, startY + 2);
	printf("Title:");
	gotoxy(startX + 20, startY + 2);
	setColor(7); // default
	printf("%s", mostPopular.title);
	gotoxy(startX, startY + 3);
	setColor(11); // cyan
	printf("Author:");
	gotoxy(startX + 20, startY + 3);
	setColor(7); // default
	printf("%s", mostPopular.author);
	gotoxy(startX, startY + 4);
	setColor(11); // cyan
	printf("Genre:");
	gotoxy(startX + 20, startY + 4);
	setColor(7); // default
	printf("%s", stringifyGenreEnum(mostPopular.genre));
	gotoxy(startX, startY + 5);
	setColor(11); // cyan
	printf("Average Rating:");
	gotoxy(startX + 20, startY + 5);
	setColor(7); // default
	if(mostPopular.avgRating != 0.0) {
		printf("%.2f", mostPopular.avgRating);
	} else {
		printf("N/A ");
	}
	gotoxy(startX, startY + 6);
	setColor(11); // cyan
	printf("Publication Date:");
	gotoxy(startX + 20, startY + 6);
	setColor(7); // default
	printf("%d/%d/%d", mostPopular.pubDate.day, mostPopular.pubDate.month, mostPopular.pubDate.year);


	// display least popular book
	gotoxy(startX + 50, startY);
	setColor(12); // red
	printf("Least popular book:");
	gotoxy(startX + 50, startY + 1);
	for(int k = 0; k < 30; k++) {
		printf("-");
	}
	gotoxy(startX + 50, startY + 2);
	printf("Title:");
	gotoxy(startX + 70, startY + 2);
	setColor(7); // default
	printf("%s", leastPopular.title);
	gotoxy(startX + 50, startY + 3);
	setColor(12); // red
	printf("Author:");
	gotoxy(startX + 70, startY + 3);
	setColor(7); // default
	printf("%s", leastPopular.author);
	gotoxy(startX + 50, startY + 4);
	setColor(12); // red
	printf("Genre:");
	gotoxy(startX + 70, startY + 4);
	setColor(7); // default
	printf("%s", stringifyGenreEnum(leastPopular.genre));
	gotoxy(startX + 50, startY + 5);
	setColor(12); // red
	printf("Average Rating:");
	gotoxy(startX + 70, startY + 5);
	setColor(7); // default
	if(leastPopular.avgRating != 0.0) {
		printf("%.2f", leastPopular.avgRating);
	} else {
		printf("N/A ");
	}
	gotoxy(startX + 50, startY + 6);
	setColor(12); // red
	printf("Publication Date:");
	gotoxy(startX + 70, startY + 6);
	setColor(7); // default
	printf("%d/%d/%d", leastPopular.pubDate.day, leastPopular.pubDate.month, leastPopular.pubDate.year);


	gotoxy(startX, startY + 8);
	next(currentUserID);
}

void updatePost(int postId, int userId) {
    time_t current_time;
    struct tm* time_info;
    char title[30], author[30];
    int rating, action, x;
    char ch;
    int postFound = 0; // Flag to track if a matching post is found
    struct communityPost post;
    char line[300];
    int numUsers = 0;

    loadUsersFromFile(users, &numUsers);

    FILE* file = fopen("community_posts.txt", "r+");
    if (file == NULL) {
        setColor(12); // red
        printf("Error opening community_posts.txt file.");
        setColor(7); // default
        return;
    }

    FILE* tempFile = fopen("temp_posts.txt", "w");
    if (tempFile == NULL) {
        setColor(12); // red
        printf("Error creating temporary file.");
        setColor(7); // default
        fclose(file);
        return;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        sscanf(line, "%d,%d,%[^,],%[^,],%[^,],%d/%d/%d,%d,%d\n",
               &post.id,
               &post.user.id,
               post.user.username,
               post.book.title,
               post.book.author,
               &post.date.day,
               &post.date.month,
               &post.date.year,
               &post.action,
               &post.rating);

        if (post.id == postId && post.user.id == userId) {
            postFound = 1;

            // Get current date and assign it to the post's date
            time(&current_time);
            time_info = localtime(&current_time);

            post.date.day = time_info->tm_mday;
            post.date.month = time_info->tm_mon + 1;  // tm_mon is 0-indexed, so add 1
            post.date.year = time_info->tm_year + 1900;  // tm_year is years since 1900

            // Store old information in variables
            strcpy(title, post.book.title);
            strcpy(author, post.book.author);

            action = post.action;

            if (action == 1) {
                post.user.dnf--;
                users[userId].dnf--;
            } else if (action == 2) {
                post.user.booksRead--;
                users[userId].booksRead--;
            } else if (action == 3) {
                post.user.tbr--;
                users[userId].tbr--;
            }

            rating = post.rating ? post.rating : 0;

            // Display old information on the right and prompts on the left
            gotoxy(40, 0);
            printf("Title: %s", title);
            gotoxy(40, 1);
            printf("Author: %s", author);
            gotoxy(40, 2);
            printf("Action: %s", stringifyActionEnum(action));
            gotoxy(0, 0);
            printf("Update action");
            gotoxy(0, 1);
            x = actionSelection(&post, userId);

            if (strcmp(stringifyActionEnum(post.action), "finished") == 0) {
                gotoxy(40, 3);
                printf("Rating: %d", rating);
                gotoxy(0, 6);
                printf("Rating:");
                gotoxy(0, 7);
                scanf("%d", &rating);
                // check the validity of rating
                while (rating <= 0 || rating > 5) {
                    gotoxy(0, 6);
                    setColor(12); // red
                    printf("Invalid input. Try again.");
                    setColor(7); // default
                    scanf("%d", &rating);
                }
                post.rating = rating;
            }

            // Get average rating
            post.book.avgRating = getAvgRating(post.book.author, post.book.title);
            addBookAvgRating();

            // Write the updated post to the temporary file
            fprintf(tempFile, "%d,%d,%s,%s,%s,%d/%d/%d,%d,%d\n",
                    post.id,
                    post.user.id,
                    post.user.username,
                    post.book.title,
                    post.book.author,
                    post.date.day,
                    post.date.month,
                    post.date.year,
                    post.action,
                    post.rating);

            saveUsersToFile(users, numUsers); // Update the user's file
        } else {
            // Write the unchanged post to the temporary file
            fputs(line, tempFile);
        }
    }

    fclose(file);
    fclose(tempFile);

    // Replace the original file with the temporary file
    if (remove("community_posts.txt") != 0) {
        setColor(12); // red
        printf("Error removing original file.");
        setColor(7); // default
        return;
    }
    if (rename("temp_posts.txt", "community_posts.txt") != 0) {
        setColor(12); // red
        printf("Error renaming temporary file.");
        setColor(7); // default
        return;
    }

    if (!postFound) {
        setColor(12); // red
        printf("You cannot modify this post.");
        setColor(7); // default
    }

    next(currentUserID);
}

void deletePost(int postId, int userId) {
	int postFound = 0;
	int numUsers = 0;
	int numBooks = 0;

	loadUsersFromFile(users, &numUsers);
	loadBooksFromFile(books, &numBooks);

    FILE* file = fopen("community_posts.txt", "r");
    FILE* tempFile = fopen("temp.txt", "w");

    if (file == NULL) {
        setColor(12); //red
        printf("Error opening community_posts.txt file.");
        setColor(7); // default
        return;
    }

    if (tempFile == NULL) {
        setColor(12); //red
        printf("Error creating temp.txt file.");
        setColor(7); // default
        fclose(file);
        return;
    }
    char line[500];

    while (fgets(line, sizeof(line), file)) {
        int currentPostId;
        int currentUserId;

        sscanf(line, "%d,%d", &currentPostId, &currentUserId);

        if (postId == currentPostId && userId == currentUserId) {
            postFound = 1;
            continue; // Skip writing this line to the temp file
        }

        fputs(line, tempFile); // Write the line to the temp file
    }

    fclose(file);
    fclose(tempFile);

	if (!postFound) {
        setColor(12); //red
        printf("You cannot modify this post.");
        setColor(7); // default
        remove("temp.txt"); // Delete the temporary file
        return;
    }

    remove("community_posts.txt");
    rename("temp.txt", "community_posts.txt");

    addBookAvgRating();
    for (int i = 0; i < sizeof(communityPosts) / sizeof(struct communityPost); i++) {
        if (postId == communityPosts[i].id && userId == communityPosts[i].user.id) {
        	for (int j = i + 1; j < sizeof(communityPosts) / sizeof(struct communityPost) - 1; j++) {
                communityPosts[j - 1] = communityPosts[j];
            }
            break; // Exit the loop once the post is deleted
        }
    }
    // delete the book if no one else entered it
    for(int k = 0; k < sizeof(books)/sizeof(struct Book); k++) {
		if(books[k].avgRating == 0.0) {
			for (int j = k + 1; j < sizeof(books) / sizeof(struct Book) - 1; j++) {
                books[j - 1] = books[j];
            }
            break; // Exit the loop once the book is deleted
		}
	}
	saveBooksToFile(books, numBooks); // the books' file
	saveUsersToFile(users, numUsers); // Update the user's file

	next(currentUserID);
}

int getNumPosts(FILE* file) {
	char line[100];
    int numPosts = 0;

    rewind(file); // Rewind the file pointer to the beginning

    while (fgets(line, sizeof(line), file) != NULL) {
        if (strlen(line) > 0) {
            numPosts++;
        }
    }

    printf("%d", numPosts);

    return numPosts;
}

int getValidPostID(const char* prompt) {
	int postID;
	printf("%s\n> ", prompt);
	scanf("%d", &postID);

	FILE* file = fopen("community_posts.txt", "r");
    if (file == NULL) {
        setColor(12); // red
        printf("Error opening community_posts.txt file.");
        setColor(7); // default
        return -1;
    }
    int numPosts = getNumPosts(file);

	while (postID < 0 || postID >= numPosts) {
		setColor(12); //red
		printf("Invalid ID. Try again.\n> ");
		setColor(7); //default
		scanf("%d", &postID);
	}

	fclose(file);

	return postID;
}

void userPrompts(int id) {
	int b;
	char ch;
	do {
		clearScreen();
		setColor(14); //yellow
		printf("\nCommand prompts:\n");
		setColor(7); //default
		printf("\033[36m[a]\033[0m Add a new entry.\n");
    	printf("\033[36m[u]\033[0m Update an existing entry.\n");
    	printf("\033[36m[d]\033[0m Delete an entry.\n");
    	printf("\033[36m[p]\033[0m Display community posts.\n");
    	printf("\033[36m[s]\033[0m Display all users.\n");
    	printf("\033[36m[b]\033[0m Display all logged books.\n");
    	printf("\033[36m[g]\033[0m Display books by genre.\n");
    	printf("\033[36m[r]\033[0m Display highest and lowest rated books.\n");
    	printf("\033[36m[f]\033[0m Display most and least popular books.\n");
    	printf("\033[36m[l]\033[0m Logout.\n");
    	printf("\033[36m[e]\033[0m Exit.\n");

		setColor(11); //cyan
    	printf("\n> ");
		ch = getch();
		setColor(7); //default
		switch(ch) {
			case 'a':
				clearScreen();
				addEntry(id);
				break;
			case 'u':
				clearScreen();
				b = getValidPostID("Insert the ID of the post you would like to change.");
				clearScreen();
				updatePost(b, id);
				break;
			case 'd':
				clearScreen();
				b = getValidPostID("Insert the ID of the post you would like to delete.");
				clearScreen();
				deletePost(b, id);
				break;
			case 'p':
				clearScreen();
				displayAllPosts(id);
				break;
			case 's':
				clearScreen();
				displayAllUsers(id);
				break;
			case 'b':
				clearScreen();
				displayAllBooks(id);
				break;
			case 'g':
				clearScreen();
				displayBooksByGenre(id);
				break;
			case 'r':
				clearScreen();
				highestAndLowestRatedBook(id);
				break;
			case 'f':
				clearScreen();
				mostAndLeastPopular(id);
				break;
			case 'l':
				clearScreen();
				intro();
				return;
			case 'e':
				exit(0);
				break;
			default:
				setColor(12); //red
				printf("Invalid command. Try again.\n");
				setColor(7); //default
				getch();
				break;
		}
	} while (1);
}

int isDateValid(int day, int month, int year) {
    if (year < 0 || year > 9999) {
        return 0; // Year outside valid range
    }
    if (month < 1 || month > 12) {
        return 0; // Month outside valid range
    }
    int daysInMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
        daysInMonth[1] = 29; // Leap year
    }
    if (day < 1 || day > daysInMonth[month-1]) {
        return 0; // Day outside valid range
    }
    return 1;
}

void saveUsersToFile(struct User users[], int numUsers) {
    FILE *file = fopen("users.txt", "w");
    if (file == NULL) {
        printf("Error opening file for writing.\n");
        return;
    }

    for (int i = 0; i < numUsers; i++) {
        fprintf(file, "%s,%d,%s,%d,%d,%d\n", users[i].username, users[i].id, users[i].country, users[i].booksRead, users[i].dnf, users[i].tbr);
    }

    fclose(file);
}

void loadUsersFromFile(struct User users[], int *numUsers) {
    FILE *file = fopen("users.txt", "r");
    if (file == NULL) {
        printf("Error opening file for reading.\n");
        return;
    }

    *numUsers = 0;
    while (fscanf(file, "%[^,],%d,%[^,],%d,%d,%d\n", users[*numUsers].username, &users[*numUsers].id,
                  users[*numUsers].country, &users[*numUsers].booksRead, &users[*numUsers].dnf, &users[*numUsers].tbr) != EOF) {
        (*numUsers)++;
    }

    fclose(file);
}

void checkUser(int a) {
    char name[20];
    int numUsers = 0;
    printf("\nInsert username:\n> ");
    scanf("%s", name);

    loadUsersFromFile(users, &numUsers);

    if (a == 1) {
        // create new user
        for (int i = 0; i <sizeof(users)/sizeof(struct User); i++) {
            // check if username already exists
            while (strcmp(tolower(name), tolower(users[i].username)) == 0) {
                setColor(12); // red
                printf("Username already exists. Insert a new username.\n> ");
                setColor(7); // default
                scanf("%s", name);
            }

            if (strcmp(users[i].username, "") == 0) {
                strcpy(users[i].username, name);
                users[i].id = i;
                printf("\nInsert country.\n> ");
                scanf("%s", users[i].country);
                int j;
                while (users[i].country[0] == '\0' || strlen(users[i].country) > 50) {
                    setColor(12); // red
                    printf("\nInvalid input. Please try again.\n> ");
                    setColor(7); // default
                    scanf("%s", users[i].country);
                }
                for (j = 0; j < strlen(users[i].country); j++) {
                    if (!isalpha(users[i].country[j])) {
                        setColor(12); // red
                        printf("\nInvalid input. Please try again.\n> ");
                        setColor(7); // default
                        scanf("%s", users[i].country);
                        j = -1; // reset the loop index to start over from the beginning of the string
                    }
                }
                setColor(7); // default
                users[i].booksRead = 0;
                users[i].dnf = 0;
                users[i].tbr = 0;

                numUsers++;
                // Save the users to file
                saveUsersToFile(users, numUsers);

                setColor(11); // cyan
                printf("New user %s with ID %d has been created successfully!\n", users[i].username, users[i].id);
                currentUserID = users[i].id;
                next(currentUserID);
                // exit loop
                break;
            }
        }
    } else {
        // get existing user
        int found = 0; // flag to indicate if a matching username is found
        for (int i = 0; i < numUsers; i++) {
            if (strcmp(tolower(name), tolower(users[i].username)) == 0) {
                found = 1;
                setColor(7); // default
                setColor(11); // cyan
                printf("%s, welcome back!\n", users[i].username);
                currentUserID = users[i].id;
                next(currentUserID);
                // exit loop
                break;
            }
        }

        if (!found) {
            setColor(12); // red
            printf("Username doesn't exist. Try again.\n");
            setColor(7); // default
        }
    }
}

void intro() {
	int a;
	char ch;

	setColor(11);  // cyan
	gotoxy(20, 1);
	for(int j = 0; j <= 40 ; j++) {
    	if(j == 0) {
    		printf("%c", 201);
		} else if(j == 40) {
			printf("%c", 187);
		} else {
			printf("%c", 205);
		}
	}

	gotoxy(20, 2);
	printf("%c", 186);
	gotoxy(35, 2);
	setColor(14); // yellow
	printf("Reading Tracker");
	setColor(11);
	gotoxy(60, 2);
	printf("%c", 186);

	gotoxy(20, 3);
	for(int j = 0; j <= 40; j++) {
    	if(j == 0) {
    		printf("%c", 200);
		} else if(j == 40) {
			printf("%c", 188);
		} else {
			printf("%c", 205);
		}
	}

	setColor(7); // default color
	gotoxy(1, 5);
	printf("Command prompts:\n");
	printf("[1] New user.\n");
    printf("[2] Existing user.\n");

	setColor(11); //cyan
    printf("\n> ");
	setColor(7); //default
	ch = getch();
	a = ch - '0';

	while(a != 1 && a != 2) {
		setColor(12); //red
		printf("Invalid command. Try again.\n> ");
		setColor(7); //default
		scanf("%d", &a);
	}
	clearScreen();
	checkUser(a);
}

int main(int argc, char *argv[]) {
	intro();

	return 0;
}
