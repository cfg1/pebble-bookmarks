#pragma once
  
#define NUM_MENU_SECTIONS 3
#define NUM_FIRST_MENU_ITEMS 10 //number of bookmarks
#define NUM_SECOND_MENU_ITEMS 1
#define NUM_THIRD_MENU_ITEMS 2
  
#define MAX_NAME_LEN    20
#define MAX_PAGE_DIGITS  4

typedef struct
{
  char name[MAX_NAME_LEN];
  int  page_number;
} book_t;


void load_settings_page(int index);