/*
 * process_cal3.c
 *
 * Starter file provided to students for Assignment #3, SENG 265,
 * Fall 2021.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "emalloc.h"
#include "ics.h"
#include "listy.h"

/*
All function names describe their purpose, same for variable names.
Anything deemed ambiguous will be explained via comment when needed.
*/

void free_all_events(node_t *list);

void free_all_nodes(node_t *list);

void create_date(int year, int month, int day, char *dt);

node_t *create_list(FILE* file, int *size);

node_t *add_event(node_t *list, char *startdt, char *enddt, char *location, char *summary, char *until, int repeating, int *size);

node_t *add_repeating(node_t *list, char *startdt, char *enddt, char *location, char *summary, char *limit, int *size);

event_t *create_event(char *startdt, char *enddt, char *location, char *summary);

void extract_time(char *time_only, char *fulldt);

void get_hr_min_int(int *hr, int *min, char* time_only);

void format_time(char *t_formatted, char *raw_time);

void format_date(char *formatted_date, const char *dt_time, const int len);

void dt_increment(char *after, const char *before, int const num_days);

int in_range(char *date, char *from, char *to);

int compare(event_t *event1, event_t *event2);

int compare_time(char *first_time, char *second_time);

node_t *sort(node_t *head);

void print_event(event_t *event, int print_full);

void print_all(node_t *list, int size, char *from, char *to);

int main(int argc, char *argv[]){

//using command line parsing from a1
    int from_y = 0, from_m = 0, from_d = 0;
    int to_y = 0, to_m = 0, to_d = 0;
    char *filename = NULL;
    int i; 

    for (i = 0; i < argc; i++) {
        if (strncmp(argv[i], "--start=", 8) == 0) {
            sscanf(argv[i], "--start=%d/%d/%d", &from_y, &from_m, &from_d);
        } else if (strncmp(argv[i], "--end=", 6) == 0) {
            sscanf(argv[i], "--end=%d/%d/%d", &to_y, &to_m, &to_d);
        } else if (strncmp(argv[i], "--file=", 7) == 0) {
            filename = argv[i]+7;
        }
    }

    
    if (from_y == 0 || to_y == 0 || filename == NULL) {
        fprintf(stderr, 
            "usage: %s --start=yyyy/mm/dd --end=yyyy/mm/dd --file=icsfile\n",
            argv[0]);
        exit(1);
    }
    
    //make CL args usable strings similar to dt for ease of use later in the program
    char from_dt[9];
    char to_dt[9];
    create_date(from_y, from_m, from_d, from_dt);
    create_date(to_y, to_m, to_d, to_dt); 
    from_dt[8] = '\0';
    to_dt[8] = '\0';

    FILE* file = fopen(filename, "r");

    //size var used for detecting edge cases in printing functions
    int size = 0;
    node_t *list = create_list(file, &size);

    node_t *sorted = sort(list);

    print_all(sorted, size, from_dt, to_dt);

    free_all_events(sorted);
    free_all_nodes(sorted);

    fclose(file);
    exit(0);
}

void free_all_events(node_t *list){

    node_t *curr = list;
    while(curr != NULL){

        free(curr->val);

        curr = curr->next;
    }
}

void free_all_nodes(node_t *list){

    node_t *curr = list;
    while(curr != NULL){
        
        node_t *next = curr->next;
        free(curr);

        curr = next;
    }
}

void create_date(int year, int month, int day, char *dt){

    /*
    The following code determines when extra zeros need to be
    added for consistancy with given DT strings
    */
    if(month > 9){
        if(day > 9){
            sprintf(dt, "%d%d%d", year, month, day);
        }else{
            sprintf(dt, "%d%d0%d", year, month, day); 
        }
    }else{
        if(day > 9){
            sprintf(dt, "%d0%d%d", year, month, day);
        }else{
            sprintf(dt, "%d0%d0%d", year, month, day); 
        }
    }
}

node_t *create_list(FILE* file, int *size){

    node_t *list = NULL;
    char startdt[16];
    char enddt[16];
    char r_untildt[16];
    char summary[SUMMARY_LEN];
    char location[LOCATION_LEN];
    int repeating = 0;

    char curr_line[132];
    while((fgets(curr_line, 132, file)) != NULL){

        int length = strlen(curr_line);

        if(strncmp(curr_line, "BEGIN:VEVENT", 12) == 0){  

            repeating = 0;

        }else if((strncmp(curr_line, "DTSTART:", 8)) == 0){

            for(int i = 0; i < 15; i++){

                startdt[i] = curr_line[(i + 8)];

            }

            startdt[15] = '\0';

        }else if((strncmp(curr_line, "DTEND:", 6)) == 0){

            for(int i = 0; i < 15; i++){

                enddt[i] = curr_line[(i + 6)];

            }

            enddt[15] = '\0';
        
        }else if((strncmp(curr_line, "RRULE:", 6)) == 0){

            for(int i = 0; i < length; i++){
                
                //prev is used to determine when to start copying important info
                char prev6[6] = "";
                if(i > 7){
                    
                    int k = 5;
                    for(int j = (i - 1); j > (i - 7); j--){
                            prev6[k] = curr_line[j];
                            k--;
                    }
                }

                if(strncmp(prev6, "UNTIL=", 6) == 0){
                        
                    for(int j = 0; j < 15; j++){

                        r_untildt[j] = curr_line[i];
                        i++;

                    }
                }  
                
            }

            r_untildt[15] = '\0';
            repeating = 1;

        }else if((strncmp(curr_line, "LOCATION:", 9)) == 0){

            int i;
            for(i = 0; i < LOCATION_LEN; i++){

                location[i] = curr_line[(i + 9)];

                if(curr_line[(i + 9)] == '\n'){
                    break;
                }

            }

            location[i] = '\0';

        }else if((strncmp(curr_line, "SUMMARY:", 8)) == 0){

            int i;
            for(i = 0; i < SUMMARY_LEN; i++){

                summary[i] = curr_line[(i + 8)];

                if(curr_line[(i + 8)] == '\n'){
                    break;
                }

            }

            summary[i] = '\0';
            
        }else if(strncmp(curr_line, "END:VEVENT", 10) == 0){

            list = add_event(list, startdt, enddt, location, summary, r_untildt, repeating, size);

        }else{

        }
    }

    return list;
}

node_t *add_event(node_t *list, char *startdt, char *enddt, char *location, char *summary, char *until, int repeating, int *size){

    event_t *new = create_event(startdt, enddt, location, summary);
    node_t *to_add = new_node(new);
    node_t *head = add_end(list, to_add);
    *size += 1;

    if(repeating == 1){

        head = add_repeating(head, startdt, enddt, location, summary, until, size);

    }

    return head;

}

node_t *add_repeating(node_t *list, char *startdt, char *enddt, char *location, char *summary, char *limit, int *size){

    char inc_start[132];
    char inc_end[132];    
    dt_increment(inc_start, startdt, 7);
    dt_increment(inc_end, enddt, 7);
    node_t *head = list;

    while(strncmp(inc_start, limit, DT_LEN) <= 0){
        
        event_t *evnt = create_event(inc_start, inc_end, location, summary);
        node_t *add_node = new_node(evnt);
        node_t *temp_head = add_end(head, add_node);
        *size += 1;
        head = temp_head;

        //incrementation of enddt for consistancy between start and end in events
        char temp_sinc[132];
        char temp_einc[132];
        dt_increment(temp_sinc, inc_start, 7);
        dt_increment(temp_einc, inc_end, 7);
        strncpy(inc_start, temp_sinc, DT_LEN);
        strncpy(inc_end, temp_einc, DT_LEN);
    }
    return head;

}

event_t *create_event(char *startdt, char *enddt, char *location, char *summary){

    event_t *new_event = (event_t*)emalloc(sizeof(event_t));
    strncpy(new_event->dtstart, startdt, DT_LEN);
    strncpy(new_event->dtend, enddt, DT_LEN);
    strncpy(new_event->location, location, LOCATION_LEN);
    strncpy(new_event->summary, summary, SUMMARY_LEN);

    return new_event;

}

void extract_time(char *time_only, char *fulldt){

    for(int i = 0; i < 6; i++){
        time_only[i] = fulldt[(i + 9)];
    }
    time_only[6] = '\0';
}

void get_hr_min_int(int *hr, int *min, char* raw_time){

    char time_only[6];
    extract_time(time_only, raw_time);

    char tempHour[2];
    char tempMin[2];

    for(int i = 0; i < 4; i++){

        if(i < 2){
            tempHour[i] = time_only[i];
        }else{
            tempMin[(i - 2)] = time_only[i];
        }

    }

    int hour = 0;
    int minute = 0;
    for(int j = 0; j < 2; j++){

        if(j == 0){
            hour += (tempHour[j] - 48) * 10;
            minute += (tempMin[j] - 48) * 10;
        }else{
            hour += tempHour[j] - 48;
            minute += tempMin[j] - 48;
        }
    }

    *hr = hour; 
    *min = minute;
}

void format_time(char *t_formatted, char *raw_time){
    
    int hour = 0;
    int min = 0;
    get_hr_min_int(&hour, &min, raw_time);

    int new_hour;
    char A_P_M[2];
    if(hour > 12){

        new_hour = hour - 12;
        A_P_M[0] = 'P';
        A_P_M[1] = 'M';

    }else{

        new_hour = hour;
        
        if(hour == 12){
            A_P_M[0] = 'P';
        }else{
            A_P_M[0] = 'A';
        }
        A_P_M[1] = 'M';

    }

    if(new_hour < 10){
        
        if(min < 10){
           sprintf(t_formatted, " %d:0%d %s", new_hour, min, A_P_M); 
        }else{
            sprintf(t_formatted, " %d:%d %s", new_hour, min, A_P_M);
        }

    }else{
        
        if(min < 10){
           sprintf(t_formatted, "%d:0%d %s", new_hour, min, A_P_M); 
        }else{
            sprintf(t_formatted, "%d:%d %s", new_hour, min, A_P_M);
        }
    }

}

void format_date(char *formatted_date, const char *dt_time, const int len){
    
    struct tm temp_time;
    time_t    full_time;

    /*  
     * Ignore for now everything other than the year, month and date.
     * For conversion to work, months must be numbered from 0, and the 
     * year from 1900.
     */  
    memset(&temp_time, 0, sizeof(struct tm));
    sscanf(dt_time, "%4d%2d%2d",
        &temp_time.tm_year, &temp_time.tm_mon, &temp_time.tm_mday);
    temp_time.tm_year -= 1900;
    temp_time.tm_mon -= 1;
    full_time = mktime(&temp_time);
    strftime(formatted_date, len, "%B %d, %Y (%a)", 
        localtime(&full_time));
}

void dt_increment(char *after, const char *before, int const num_days){
    struct tm temp_time;
    time_t    full_time;

    memset(&temp_time, 0, sizeof(struct tm));
    sscanf(before, "%4d%2d%2dT%2d%2d%2d", &temp_time.tm_year,
        &temp_time.tm_mon, &temp_time.tm_mday, &temp_time.tm_hour, 
        &temp_time.tm_min, &temp_time.tm_sec);
    temp_time.tm_year -= 1900;
    temp_time.tm_mon -= 1;
    temp_time.tm_mday += num_days;

    full_time = mktime(&temp_time);
    after[0] = '\0';
    strftime(after, 16, "%Y%m%dT%H%M%S", localtime(&full_time));
    strncpy(after + 16, before + 16, (132 - 16)); 
    after[132 - 1] = '\0';
}

/*
the following three functions use strncmp to compare two DT strings
lengths are provideed as args to keep operations safe
*/
int in_range(char *date, char *from, char *to){

    char date_only[9];
    strncpy(date_only, date, 8);
    date_only[8] = '\0';

    if((strncmp(date_only, from, 8) >= 0) && (strncmp(date_only, to, 8) <= 0)){
        return 1;
    }else{
        return 0;
    }
}

//will return 1 if event 1 is befrore event 2, -1 if event 2 is before event 1 and 0 if they are equal
int compare(event_t *event1, event_t *event2){

    char first_date[9];
    char second_date[9];
    strncpy(first_date, event1->dtstart, 8);
    strncpy(second_date, event2->dtstart, 8);
    first_date[8] = '\0';
    second_date[8] = '\0';

    if(strncmp(first_date, second_date, 8) >= 0){
        
        if(strncmp(first_date, second_date, 8) == 0){

            if(compare_time(event1->dtstart, event2->dtstart) == 1){
                return 1;
            }else if(compare_time(event1->dtstart, event2->dtstart) == 0){
                return 0;
            }else{
                return -1;
            }
        }else{
            return 1;
        }

    }else{
        return -1;
    }


}

//same behavior for return value as compare function above
int compare_time(char *first_time, char *second_time){

    int hr1 = 0;
    int min1 = 0;
    int hr2 = 0;
    int min2 = 0;
    get_hr_min_int(&hr1, &min1, first_time);
    get_hr_min_int(&hr2, &min2, second_time);

    if(hr1 >= hr2){
        if(hr1 == hr2){

            if(min1 >= min2){

                if(min1 == min2){
                    return 0;
                }else{
                    return 1;
                }

            }else{
                return -1;
            }
        }else{
            return 1;
        }
    }else{
        return -1;
    }
    
}

//sort via a bubble sort algorithm
node_t *sort(node_t *head){

    if(head == NULL || head->next == NULL){
        
        //empty list or list with 1 element is already sorted 
        return head;
    
    }
    
    node_t *curr = head;
    while(curr->next != NULL){

        node_t *comp_node = head;
        while(comp_node->next != NULL){

            if(compare(comp_node->val, curr->val) == 1){

                event_t *temp = curr->val;
                curr->val = comp_node->val;
                comp_node->val = temp;

            }

            comp_node = comp_node->next;

        }

        curr = curr->next;
    }

    return head;
    
}

void print_all(node_t *list, int size, char *from, char *to){

    node_t *curr = list;
    
    //checks for edge case when there is only one event in the list
    if(size == 1){
        print_event(curr->val, 1);
    }else{

        while(curr != NULL){

            event_t *event = curr->val;

            if(in_range(event->dtstart, from, to)){

                print_event(event, 1);
                
                //if statement keeps the function from causing a segmentation fault
                if(curr->next != NULL){
                    if(strncmp(curr->next->val->dtstart, curr->val->dtstart, 8) == 0){

                        char date[9];
                        strncpy(date, curr->val->dtstart, 8);
                        curr = curr->next;
                        
                        while(strncmp(curr->val->dtstart, date, 8) == 0){
                           
                            print_event(curr->val, 0);
                            
                            /*
                            to prevent over-indexing of the curr varible as this 
                            causes the printing to miss a few events
                            */
                            if(strncmp(curr->next->val->dtstart, date, 8) == 0){
                                curr = curr->next;
                            }else{
                                break;
                            }
                        }
                    }
                }

                //makes sure an empty line between events is only printed when needed
                if((curr->next != NULL) && in_range(curr->next->val->dtstart, from, to)){
                    printf("\n");
                }
            }
            
            curr = curr->next;
        }
    }
}

void print_event(event_t *event, int print_full){

    char start[132];
    char end[132];
    format_time(start, event->dtstart);
    format_time(end, event->dtend);
    
    char formatted_date[132];
    format_date(formatted_date, event->dtstart, 132);
    
    if(print_full == 1){

        printf("%s\n", formatted_date);

        for(int i = 0; i < strlen(formatted_date); i++){
            printf("-");
        }
        printf("\n");

        printf("%s to %s: %s {{%s}}\n", start, end, event->summary, event->location);

    }else{
        printf("%s to %s: %s {{%s}}\n", start, end, event->summary, event->location); 
    }
}