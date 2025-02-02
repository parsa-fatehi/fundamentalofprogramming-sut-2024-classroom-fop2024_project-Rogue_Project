//403106379
#include <ncurses.h>
#include <menu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_SAVE_SLOTS 10
#define MAX_LEADERBOARD_ENTRIES 100
#define MAX_USERS 100
#define FILENAME "users.dat"

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAP_WIDTH  80
#define MAP_HEIGHT 24
#define ROOM_MIN_SIZE 5
#define ROOM_MAX_SIZE 10
#define MAX_ROOMS 8
#define PLAYER_CHAR '@'
#define DOOR_CHAR '+'
#define TRAP_CHAR '^'
#define STAIR_CHAR '<'
#define PLAYER_START_HEALTH 10
#define SAVE_FILE "game_save.txt"


typedef struct {
    int x, y, width, height;
    int door_x1, door_y1;      
    int door_x2, door_y2; 
    int hidden_door_x, hidden_door_y; 
    int hidden_door_visible;      
} Room;

typedef struct {
    char map[MAP_HEIGHT][MAP_WIDTH];  
    int revealed_traps[MAP_HEIGHT][MAP_WIDTH];
    Room rooms[MAX_ROOMS];           
    int room_count;                  
    int player_x, player_y;         
    int player_health; 
    int stair_x, stair_y;
    int level; 
} Dungeon;

void init_map(Dungeon *dungeon) {
    int i, j;
    for (i = 0; i < MAP_HEIGHT; i++) {
        for (j = 0; j < MAP_WIDTH; j++) {
            dungeon->map[i][j] = ' ';
            dungeon->revealed_traps[i][j] = 0;
        }
    }
    dungeon->room_count = 0;
    dungeon->level = 1;  
}

int create_room(Dungeon *dungeon) {
    if (dungeon->room_count >= MAX_ROOMS) return 0;

    int width = ROOM_MIN_SIZE + rand() % (ROOM_MAX_SIZE - ROOM_MIN_SIZE);
    int height = ROOM_MIN_SIZE + rand() % (ROOM_MAX_SIZE - ROOM_MIN_SIZE);
    int x = 2 + rand() % (MAP_WIDTH - width - 4);
    int y = 2 + rand() % (MAP_HEIGHT - height - 4);
    int i;
    
    for (i = 0; i < dungeon->room_count; i++) {
        Room *r = &dungeon->rooms[i];
        if (x < r->x + r->width + 2 && x + width + 2 > r->x &&
            y < r->y + r->height + 2 && y + height + 2 > r->y)
            return 0;
    }
    

    for (i = y; i < y + height; i++) {
        int j;
        for (j = x; j < x + width; j++) {
            dungeon->map[i][j] = '.';
        }
    }
   
    for (i = x; i < x + width; i++) {
        dungeon->map[y][i] = dungeon->map[y + height - 1][i] = '-';
    }
   
    for (i = y; i < y + height; i++) {
        dungeon->map[i][x] = dungeon->map[i][x + width - 1] = '|';
    }
    
    int door_x1 = x + 1 + rand() % (width - 2);
    int door_y1 = y;
    dungeon->map[door_y1][door_x1] = DOOR_CHAR;
    
    Room new_room;
    if (dungeon->room_count < 3) {
      
        int door_x2 = -1, door_y2 = -1;
        int hidden_door_x = x + 1 + rand() % (width - 2);
        int hidden_door_y = y + 1 + rand() % (height - 2);
        int hidden_door_visible = 0; 
        new_room.x = x; new_room.y = y; new_room.width = width; new_room.height = height;
        new_room.door_x1 = door_x1; new_room.door_y1 = door_y1;
        new_room.door_x2 = door_x2; new_room.door_y2 = door_y2;
        new_room.hidden_door_x = hidden_door_x; new_room.hidden_door_y = hidden_door_y;
        new_room.hidden_door_visible = hidden_door_visible;
        
        int num_traps = 1;  
        for (i = 0; i < num_traps; i++) {
            int trap_x = x + 1 + rand() % (width - 2);
            int trap_y = y + 1 + rand() % (height - 2);
            dungeon->map[trap_y][trap_x] = TRAP_CHAR;
            dungeon->revealed_traps[trap_y][trap_x] = 0;
        }
    } else {
        int door_x2, door_y2;
        if (rand() % 2 == 0) {
            if (door_x1 - x < width / 2)
                door_x2 = x + width - 1;
            else
                door_x2 = x;
            door_y2 = y + 1 + rand() % (height - 2);
        } else {
            door_x2 = x + 1 + rand() % (width - 2);
            door_y2 = y + height - 1;
        }
        dungeon->map[door_y2][door_x2] = DOOR_CHAR;
        
        new_room.x = x; new_room.y = y; new_room.width = width; new_room.height = height;
        new_room.door_x1 = door_x1; new_room.door_y1 = door_y1;
        new_room.door_x2 = door_x2; new_room.door_y2 = door_y2;
        new_room.hidden_door_x = -1; new_room.hidden_door_y = -1;
        new_room.hidden_door_visible = 1;
        
        int num_traps = rand() % 2;
        for (i = 0; i < num_traps; i++) {
            int trap_x = x + 1 + rand() % (width - 2);
            int trap_y = y + 1 + rand() % (height - 2);
            dungeon->map[trap_y][trap_x] = TRAP_CHAR;
            dungeon->revealed_traps[trap_y][trap_x] = 0;
        }
    }
    
    if (dungeon->room_count == MAX_ROOMS - 1) {
        dungeon->stair_x = x + width / 2;
        dungeon->stair_y = y + height / 2;
        dungeon->map[dungeon->stair_y][dungeon->stair_x] = STAIR_CHAR;
    }
    
    dungeon->rooms[dungeon->room_count++] = new_room;
    return 1;
}

void place_player(Dungeon *dungeon) {
    dungeon->player_x = dungeon->rooms[0].x + 1;
    dungeon->player_y = dungeon->rooms[0].y + 1;
}

void display_map(Dungeon *dungeon) {
    int i, j;
    clear();
    for (i = 0; i < MAP_HEIGHT; i++) {
        for (j = 0; j < MAP_WIDTH; j++) {
            char tile = dungeon->map[i][j];
            if (tile == TRAP_CHAR && !dungeon->revealed_traps[i][j])
                mvaddch(i, j, '.');
            else
                mvaddch(i, j, tile);
        }
    }
    mvprintw(MAP_HEIGHT, 0, "Health: %d  Level: %d", dungeon->player_health, dungeon->level);
    mvaddch(dungeon->player_y, dungeon->player_x, PLAYER_CHAR);
    refresh();
}

void move_player(Dungeon *dungeon, int dx, int dy) {
    int new_x = dungeon->player_x + dx;
    int new_y = dungeon->player_y + dy;
    char tile = dungeon->map[new_y][new_x];

    if (tile == '.' || tile == '+' || tile == ' ' || tile == TRAP_CHAR || tile == STAIR_CHAR) {
        dungeon->player_x = new_x;
        dungeon->player_y = new_y;

        if (tile == TRAP_CHAR) {
            dungeon->player_health--;
            dungeon->revealed_traps[new_y][new_x] = 1;
            mvprintw(MAP_HEIGHT + 1, 0, "You stepped on a trap!");
            if (dungeon->player_health <= 0) {
                mvprintw(MAP_HEIGHT + 2, 0, "Game Over! You ran out of health.");
                refresh();
                getch();
                endwin();
                exit(0);
            }
        }

        if (tile == STAIR_CHAR) {
            if (dungeon->level == 1) {
                dungeon->level = 2;
                mvprintw(MAP_HEIGHT + 1, 0, "Level 2: You've moved to the second level!");
            } else {
                dungeon->level = 1;
                mvprintw(MAP_HEIGHT + 1, 0, "Level 1: You've moved back to the first level!");
            }
            refresh();
            getch();
        }
    }
}
void newgame() {
    Dungeon dungeon;
    int health_choice;
    
    clear();
    mvprintw(0, 0, "Select difficulty level (1: Easy, 2: Medium, 3: Hard): ");
    int ch = getch();
    if (ch == '1') {
        health_choice = 30;
    } else if (ch == '2') {
        health_choice = 20;
    } else {
        health_choice = 10;
    }

    dungeon.player_health = health_choice;
    srand(time(NULL));
    initscr();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);

    init_map(&dungeon);
    int attempts = 0;
    while (dungeon.room_count < MAX_ROOMS && attempts < MAX_ROOMS * 5) {
        create_room(&dungeon);
        attempts++;
    }

    place_player(&dungeon);
    display_map(&dungeon);

    int move_ch;
    while ((move_ch = getch()) != 'q') {
        switch (move_ch) {
                   case KEY_UP:    move_player(&dungeon, 0, -1); break;
            case KEY_DOWN:  move_player(&dungeon, 0, 1); break;
            case KEY_LEFT:  move_player(&dungeon, -1, 0); break;
            case KEY_RIGHT: move_player(&dungeon, 1, 0); break;
            case 56:        move_player(&dungeon, 0, -1); break;
            case 50:        move_player(&dungeon, 0, 1); break;
            case 52:        move_player(&dungeon, -1, 0); break;
            case 54:        move_player(&dungeon, 1, 0); break;
            case 55:        move_player(&dungeon, -1, -1); break;
            case 57:        move_player(&dungeon, 1, -1); break;
            case 51:        move_player(&dungeon, 1, 1); break;
            case 49:        move_player(&dungeon, -1, 1); break;
        }
        display_map(&dungeon);
    }

    endwin();
}
typedef struct {
    char username[50];
    char password[50];
    char email[50];
    int birth_day;
    int birth_month;
} User;
typedef struct {
    char username[50];
    int score;
    int gold;
    int games_completed;
    time_t first_game_time;
} Player;

typedef struct {
    char save_name[50];
    int progress;
    int score;
} SaveGame;


Player leaderboard[MAX_LEADERBOARD_ENTRIES];
int leaderboard_count = 0;
SaveGame saved_games[MAX_SAVE_SLOTS];
int save_count = 0;
User users[MAX_USERS];
int user_count = 0;

void display_pre_game_menu();
void new_game();
void continue_game();
void show_leaderboard();
void settings_menu();
void profile_menu();
void save_game(const char *save_name, int progress, int score);
void load_leaderboard();
void display_leaderboard_entry(int index, int is_current_user);
void display_settings();


int is_valid_password(const char *password);
int is_valid_email(const char *email);
int username_exists(const char *username);
void save_user(const User *user);
void load_users(User users[], int *user_count);
void display_main_menu();
void user_menu(const char *username);
void new_user_menu();
void login_menu();
void forgot_password_menu(const char *username);
char *generate_random_password();
int is_valid_passwordlen(const char *password);

void display_main_menu() {
    ITEM *items[4];
    MENU *menu;
    int c;

    items[0] = new_item("1. Create New User", "");
    items[1] = new_item("2. Login", "");
    items[2] = new_item("3. Forgot Password", "");
    items[3] = new_item("4. Exit", "");
    items[4] = NULL;

    menu = new_menu((ITEM **)items);

    clear();
    mvprintw(LINES - 3, 0, "Use arrow keys to navigate, Enter to select.");
    post_menu(menu);
    refresh();

    while ((c = getch()) != '\n') {
        switch (c) {
            case KEY_DOWN:
                menu_driver(menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(menu, REQ_UP_ITEM);
                break;
        }
    }

    ITEM *cur = current_item(menu);
    if (strcmp(item_name(cur), "1. Create New User") == 0) {
        unpost_menu(menu);
        for (int i = 0; i < 4; i++) free_item(items[i]);
        free_menu(menu);
        new_user_menu();
    } else if (strcmp(item_name(cur), "2. Login") == 0) {
        unpost_menu(menu);
        for (int i = 0; i < 4; i++) free_item(items[i]);
        free_menu(menu);
        login_menu();
    } else if (strcmp(item_name(cur), "3. Forgot Password") == 0) {
        unpost_menu(menu);
        for (int i = 0; i < 4; i++) free_item(items[i]);
        free_menu(menu);
        forgot_password_menu(NULL);
    } else if (strcmp(item_name(cur), "4. Exit") == 0) {
        unpost_menu(menu);
        for (int i = 0; i < 4; i++) free_item(items[i]);
        free_menu(menu);
        endwin();
        exit(0);
    }
}


void new_user_menu() {
    char username[50], password[50], email[50];
    int day = 0, month = 0;

    while (1) {
        clear();
        mvprintw(2, 2, "New User Registration");
        mvprintw(4, 2, "Enter username: ");
        echo();
        getstr(username);

        if (username_exists(username)) {
            mvprintw(6, 2, "Error: Username already exists!");
            getch();
            continue;
        }

        mvprintw(5, 2, "Enter password (or press 1 for a random password): ");
        getstr(password);

        if (strcmp(password, "1") == 0) {
            char *random_password = generate_random_password();
            strcpy(password, random_password);
            free(random_password);
            mvprintw(6, 2, "Generated password: %s", password);
            getch();
        } else if (!is_valid_password(password)) {
            mvprintw(6, 2, "Error: Password must include a number, a lowercase, and an uppercase letter.");
            getch();
            continue;
        }
        else if (!is_valid_passwordlen(password)) {
            mvprintw(6, 2, "Error: Password must be at least 7 characters");
            getch();
            continue;
        }
        


        mvprintw(7, 2, "Enter email: ");
        getstr(email);

        if (!is_valid_email(email)) {
            mvprintw(8, 2, "Error: Invalid email format. Must be in zzz.y@xxx format.");
            getch();
            continue;
        }

        mvprintw(9, 2, "Optional: Enter your birth day (1-31) for password recovery or leave empty: ");
        char day_str[10];
        getstr(day_str);
        if (strlen(day_str) > 0) day = atoi(day_str);

        mvprintw(10, 2, "Optional: Enter your birth month (1-12) or leave empty: ");
        char month_str[10];
        getstr(month_str);
        if (strlen(month_str) > 0) month = atoi(month_str);

        User new_user;
        strcpy(new_user.username, username);
        strcpy(new_user.password, password);
        strcpy(new_user.email, email);
        new_user.birth_day = day;
        new_user.birth_month = month;

        save_user(&new_user);
        mvprintw(11, 2, "User created successfully!");
        getch();
         display_pre_game_menu();
        return;
    }
}

void login_menu() {
    char username[50], password[50];
    while (1) {
        clear();
        mvprintw(2, 2, "User Login");
        mvprintw(4, 2, "Enter username: ");
        echo();
        getstr(username);

        mvprintw(5, 2, "Enter password: ");
        getstr(password);

        for (int i = 0; i < user_count; i++) {
            if (strcmp(users[i].username, username) == 0) {
                if (strcmp(users[i].password, password) == 0) {
                     display_pre_game_menu();
                    return;
                } else {
                    mvprintw(7, 2, "Error: Incorrect password.");
                    mvprintw(8, 2, "Press R to recover password or any other key to retry.");
                    char choice = getch();
                    if (choice == 'R' || choice == 'r') {
                        forgot_password_menu(username);
                        return;
                    }
                    continue;
                }
            }
        }

        mvprintw(7, 2, "Error: Username not found.");
        getch();
    }
}

void forgot_password_menu(const char *username) {
    clear();
    mvprintw(2, 2, "Password Recovery");

    if (username == NULL) {
        mvprintw(4, 2, "Enter username for recovery: ");
        char temp_username[50];
        echo();
        getstr(temp_username);
        username = temp_username;
    }

    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            if (users[i].birth_day == 0 || users[i].birth_month == 0) {
                mvprintw(5, 2, "Error: No recovery information available for this user.");
                getch();
                return;
            }

            mvprintw(6, 2, "Enter your birth day: ");
            int day;
            scanw("%d", &day);

            mvprintw(7, 2, "Enter your birth month: ");
            int month;
            scanw("%d", &month);

            if (day == users[i].birth_day && month == users[i].birth_month) {
                mvprintw(9, 2, "Your password is: %s", users[i].password);
            } else {
                mvprintw(9, 2, "Error: Incorrect recovery information.");
            }
            getch();
            return;
        }
    }

    mvprintw(5, 2, "Error: Username not found.");
    getch();
}


int is_valid_passwordlen(const char *password) {
    if (strlen(password) < 7) return 0;
    else
    return 1;
}
int is_valid_password(const char *password) {
    if (strlen(password) < 7) return 0;
    int has_upper = 0, has_lower = 0, has_digit = 0;

    for (int i = 0; password[i]; i++) {
        if (isupper(password[i])) has_upper = 1;
        if (islower(password[i])) has_lower = 1;
        if (isdigit(password[i])) has_digit = 1;
    }

    return has_upper && has_lower && has_digit;
}

int is_valid_email(const char *email) {
    const char *at = strchr(email, '@');
    if (!at || at == email || strchr(at + 1, '@')) return 0;

    const char *dot = strchr(at + 1, '.');
    if (!dot || dot == at + 1 || dot[1] == '\0') return 0;

    return 1;
}

int username_exists(const char *username) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) return 1;
    }
    return 0;
}

void save_user(const User *user) {
    FILE *file = fopen(FILENAME, "ab");
    if (!file) {
        perror("Failed to open file");
        return;
    }
    fwrite(user, sizeof(User), 1, file);
    fclose(file);

    users[user_count++] = *user;
}

void load_users(User users[], int *user_count) {
    FILE *file = fopen(FILENAME, "rb");
    if (!file) return;

    while (fread(&users[*user_count], sizeof(User), 1, file)) {
        (*user_count)++;
    }
    fclose(file);
}

char *generate_random_password() {
    static const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    char *password = malloc(9);
    if (!password) return NULL;

    srand(time(NULL));
    for (int i = 0; i < 8; i++) {
        password[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    password[8] = '\0';

    return password;
}

void display_pre_game_menu() {
    ITEM *items[6];
    MENU *menu;
    int c;

    items[0] = new_item("1. New Game", "");
    items[1] = new_item("2. Continue Previous Game", "");
    items[2] = new_item("3. Leaderboard", "");
    items[3] = new_item("4. Settings", "");
    items[4] = new_item("5. Profile", "");
    items[5] = NULL;

    menu = new_menu((ITEM **)items);

    clear();
    mvprintw(LINES - 3, 0, "Use arrow keys to navigate, Enter to select.");
    post_menu(menu);
    refresh();

    while ((c = getch()) != '\n') {
        switch (c) {
            case KEY_DOWN:
                menu_driver(menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(menu, REQ_UP_ITEM);
                break;
        }
    }

    ITEM *cur = current_item(menu);
    if (strcmp(item_name(cur), "1. New Game") == 0) {
        unpost_menu(menu);
        for (int i = 0; i < 5; i++) free_item(items[i]);
        free_menu(menu);
        new_game();
    } else if (strcmp(item_name(cur), "2. Continue Previous Game") == 0) {
        unpost_menu(menu);
        for (int i = 0; i < 5; i++) free_item(items[i]);
        free_menu(menu);
        continue_game();
    } else if (strcmp(item_name(cur), "3. Leaderboard") == 0) {
        unpost_menu(menu);
        for (int i = 0; i < 5; i++) free_item(items[i]);
        free_menu(menu);
        show_leaderboard();
    } else if (strcmp(item_name(cur), "4. Settings") == 0) {
        unpost_menu(menu);
        for (int i = 0; i < 5; i++) free_item(items[i]);
        free_menu(menu);
        settings_menu();
    } else if (strcmp(item_name(cur), "5. Profile") == 0) {
        unpost_menu(menu);
        for (int i = 0; i < 5; i++) free_item(items[i]);
        free_menu(menu);
        profile_menu();
    }
}


void new_game() {
    clear();
    mvprintw(2, 2, "Starting a new game...");

    refresh();
    getch();
           newgame();

}


void continue_game() {
    clear();
    if (save_count == 0) {
        mvprintw(2, 2, "No saved games available.");
    } else {
        mvprintw(2, 2, "Saved games:");
        for (int i = 0; i < save_count; i++) {
            mvprintw(4 + i, 4, "%d. %s (Progress: %d%%, Score: %d)", i + 1, saved_games[i].save_name, saved_games[i].progress, saved_games[i].score);
        }
    }
    refresh();
    getch();
}


void show_leaderboard() {
    clear();
    mvprintw(2, 2, "Leaderboard:");
    for (int i = 0; i < leaderboard_count && i < 10; i++) {
        display_leaderboard_entry(i, 0);
    }
    refresh();
    getch();
}

void display_leaderboard_entry(int index, int is_current_user) {
    if (is_current_user) {
        attron(A_BOLD);
    }
    mvprintw(4 + index, 2, "%d. %s | Score: %d | Gold: %d | Games: %d | Experience: %ld days",
             index + 1, leaderboard[index].username, leaderboard[index].score, leaderboard[index].gold,
             leaderboard[index].games_completed,
             (time(NULL) - leaderboard[index].first_game_time) / (60 * 60 * 24));
    if (is_current_user) {
        attroff(A_BOLD);
    }
}


void settings_menu() {
    clear();
    mvprintw(2, 2, "Settings Menu");
    display_settings();
    refresh();
    getch();
}

void display_settings() {
    mvprintw(4, 2, "1. Set difficulty level");
    mvprintw(5, 2, "2. Change character color");
    mvprintw(6, 2, "3. Choose background music");
}

void profile_menu() {
    clear();
    mvprintw(2, 2, "Profile Menu");
 
    refresh();
    getch();
}


void save_game(const char *save_name, int progress, int score) {
    if (save_count < MAX_SAVE_SLOTS) {
        strcpy(saved_games[save_count].save_name, save_name);
        saved_games[save_count].progress = progress;
        saved_games[save_count].score = score;
        save_count++;
    }
}


void load_leaderboard() {
    strcpy(leaderboard[0].username, "Player1");
    leaderboard[0].score = 5000;
    leaderboard[0].gold = 100;
    leaderboard[0].games_completed = 50;
    leaderboard[0].first_game_time = time(NULL) - (60 * 60 * 24 * 365);

    strcpy(leaderboard[1].username, "Player2");
    leaderboard[1].score = 3000;
    leaderboard[1].gold = 50;
    leaderboard[1].games_completed = 30;
    leaderboard[1].first_game_time = time(NULL) - (60 * 60 * 24 * 200);

    leaderboard_count = 2;
}

int main() {
    load_users(users, &user_count);

    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);

    display_main_menu();
   load_leaderboard();
    endwin();
    return 0;
}
   