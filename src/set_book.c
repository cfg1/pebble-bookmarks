#include <pebble.h>
#include "set_book.h"
  
#define EDIT_FIELD_WIDTH  25
#define EDIT_FIELD_HEIGHT 35
#define EDIT_FIELD_Y_POS  60

#define EDIT_X_1  ((int)((144-EDIT_FIELD_WIDTH)/2))
#define EDIT_X_2  ((int)((144+EDIT_FIELD_WIDTH)/2))
#define EDIT_Y_2  EDIT_FIELD_HEIGHT+EDIT_FIELD_Y_POS


Window* window_change_entry;

TextLayer* text_layer_edit; //This TextLayer changes its size so that it is one char in edit_mode_name and covors the full width on all other modes.
//these 7 layers are only used in edit_mode_name:
TextLayer* text_layer_edit_left;
TextLayer* text_layer_edit_right;
TextLayer* text_layer_edit_top;
TextLayer* text_layer_edit_bottom;
TextLayer* text_layer_edit_top_2;
TextLayer* text_layer_edit_bottom_2;
TextLayer* text_layer_edit_char_pos;

/*
char msg[10+1+6+1];
*/
static char int2str[20];
static int selected_index; // intern, is received through load_settings_page()
//extern vars are set before load_settings_page():
extern book_t book_list[]; // Defined in main.c
extern int edit_mode_name;
extern int clear_mode_single_item;
extern int clear_mode_all;

static int edit_pos = 0;


static void click_config_provider(void *context);
static char next_char(char c, int updown);


static void save_item(int index){
  int persist_start_index = index*2;
  persist_write_string(persist_start_index,  book_list[index].name);
  persist_write_int(persist_start_index + 1, book_list[index].page_number);
}

void display_edit_fields(){
  if (!edit_mode_name) return;
  static char str_left[20];
  static char str_right[20];
  static char str_top[5];
  static char str_bottom[5];
  static char str_top_2[5];
  static char str_bottom_2[5];
  static char str_char_pos[20];
  
  int i,j;
  snprintf(int2str, sizeof(int2str), "%c", book_list[selected_index].name[edit_pos]);
  j = 0;
  for (i=0;i<edit_pos;i++){
    str_left[j] = book_list[selected_index].name[i];
    j++;
  }
  if (j == 0){
    str_left[j] = '\0';
  } else {
    if (j<MAX_NAME_LEN) if (str_left[j-1] != '\0') str_left[j] = '\0';
  }
  //strncpy(str_left, book_list[selected_index].name, edit_pos);
  j = 0;
  for (i=edit_pos+1; i<MAX_NAME_LEN; i++){
    str_right[j] = book_list[selected_index].name[i];
    j++;
  }
  if (j == 0){
    str_right[j] = '\0';
  } else {
    if (j<MAX_NAME_LEN) if (str_right[j-1] != '\0') str_right[j] = '\0';
  }
  snprintf(str_top,    sizeof(str_top),    "%c", next_char(book_list[selected_index].name[edit_pos], -1));
  snprintf(str_bottom, sizeof(str_bottom), "%c", next_char(book_list[selected_index].name[edit_pos],  1));
  snprintf(str_top_2,    sizeof(str_top),    "%c", next_char(book_list[selected_index].name[edit_pos], -2));
  snprintf(str_bottom_2, sizeof(str_bottom), "%c", next_char(book_list[selected_index].name[edit_pos],  2));
  
  snprintf(str_char_pos, sizeof(str_char_pos), "%d/%d", edit_pos+1, MAX_NAME_LEN);
  /*
  strcpy(str_left, "Begin");
  strcpy(str_right, "End");
  strcpy(str_top, "X");
  strcpy(str_bottom, "Y");
  */
  
  text_layer_set_text(text_layer_edit_left,   str_left);
  text_layer_set_text(text_layer_edit_right,  str_right);
  text_layer_set_text(text_layer_edit_top,    str_top);
  text_layer_set_text(text_layer_edit_bottom, str_bottom);
  text_layer_set_text(text_layer_edit_top_2,    str_top_2);
  text_layer_set_text(text_layer_edit_bottom_2, str_bottom_2);
  text_layer_set_text(text_layer_edit_char_pos, str_char_pos);
}



void unload_settings_page(Window* win){
  save_item(selected_index);
  text_layer_destroy(text_layer_edit);
  if (edit_mode_name){
    text_layer_destroy(text_layer_edit_left);
    text_layer_destroy(text_layer_edit_right);
    text_layer_destroy(text_layer_edit_top);
    text_layer_destroy(text_layer_edit_bottom);
    text_layer_destroy(text_layer_edit_top_2);
    text_layer_destroy(text_layer_edit_bottom_2);
    text_layer_destroy(text_layer_edit_char_pos);
  }
  window_destroy(window_change_entry);
}

void load_settings_page(int index){
  if (index>=0) selected_index = index;
  
  WindowHandlers wh = { .unload = &unload_settings_page };
  window_change_entry = window_create();
  window_set_click_config_provider(window_change_entry, click_config_provider);
  window_set_window_handlers(window_change_entry, wh);
  
  if (edit_mode_name){
    // --- TODO ---
    /*
    int edit_x_1 = (144-EDIT_FIELD_WIDTH)/2;
    int edit_x_2 = (int)((144+EDIT_FIELD_WIDTH)/2);
    int edit_y_2 = EDIT_FIELD_HEIGHT+EDIT_FIELD_Y_POS;
    */
    text_layer_edit = text_layer_create(GRect(EDIT_X_1, EDIT_FIELD_Y_POS, EDIT_FIELD_WIDTH, EDIT_FIELD_HEIGHT));
    //text_layer_edit = text_layer_create(GRect(52, 112, 40, 60));
    //text_layer_edit = text_layer_create(GRect(edit_x_1, EDIT_FIELD_Y_POS, EDIT_FIELD_WIDTH, EDIT_FIELD_HEIGHT));
    text_layer_set_text_alignment(text_layer_edit, GTextAlignmentCenter); // Center the text.
    text_layer_set_font(text_layer_edit, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_background_color(text_layer_edit, GColorBlack);
    text_layer_set_text_color(text_layer_edit, GColorWhite);
    
    text_layer_edit_left = text_layer_create(GRect(0, EDIT_FIELD_Y_POS, EDIT_X_1, EDIT_FIELD_HEIGHT));
    text_layer_set_text_alignment(text_layer_edit_left, GTextAlignmentRight);
    text_layer_set_font(text_layer_edit_left, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_background_color(text_layer_edit_left, GColorWhite);
    text_layer_set_text_color(text_layer_edit_left, GColorBlack);
    
    text_layer_edit_right = text_layer_create(GRect(EDIT_X_2, EDIT_FIELD_Y_POS, 144-EDIT_X_2, EDIT_FIELD_HEIGHT));
    text_layer_set_text_alignment(text_layer_edit_right, GTextAlignmentLeft);
    text_layer_set_font(text_layer_edit_right, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_background_color(text_layer_edit_right, GColorWhite);
    text_layer_set_text_color(text_layer_edit_right, GColorBlack);
    
    text_layer_edit_top = text_layer_create(GRect(EDIT_X_1, EDIT_FIELD_Y_POS-EDIT_FIELD_HEIGHT+5, EDIT_FIELD_WIDTH, EDIT_FIELD_HEIGHT));
    text_layer_set_text_alignment(text_layer_edit_top, GTextAlignmentCenter);
    text_layer_set_font(text_layer_edit_top, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_background_color(text_layer_edit_top, GColorClear);
    text_layer_set_text_color(text_layer_edit_top, GColorBlack);
    
    text_layer_edit_bottom = text_layer_create(GRect(EDIT_X_1, EDIT_FIELD_Y_POS+EDIT_FIELD_HEIGHT-5, EDIT_FIELD_WIDTH, EDIT_FIELD_HEIGHT));
    text_layer_set_text_alignment(text_layer_edit_bottom, GTextAlignmentCenter);
    text_layer_set_font(text_layer_edit_bottom, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_background_color(text_layer_edit_bottom, GColorClear);
    text_layer_set_text_color(text_layer_edit_bottom, GColorBlack);
    
    text_layer_edit_top_2 = text_layer_create(GRect(EDIT_X_1, EDIT_FIELD_Y_POS-2*EDIT_FIELD_HEIGHT+10, EDIT_FIELD_WIDTH, EDIT_FIELD_HEIGHT));
    text_layer_set_text_alignment(text_layer_edit_top_2, GTextAlignmentCenter);
    text_layer_set_font(text_layer_edit_top_2, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_background_color(text_layer_edit_top_2, GColorClear);
    text_layer_set_text_color(text_layer_edit_top_2, GColorBlack);
    
    text_layer_edit_bottom_2 = text_layer_create(GRect(EDIT_X_1, EDIT_FIELD_Y_POS+2*EDIT_FIELD_HEIGHT-10, EDIT_FIELD_WIDTH, EDIT_FIELD_HEIGHT));
    text_layer_set_text_alignment(text_layer_edit_bottom_2, GTextAlignmentCenter);
    text_layer_set_font(text_layer_edit_bottom_2, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_background_color(text_layer_edit_bottom_2, GColorClear);
    text_layer_set_text_color(text_layer_edit_bottom_2, GColorBlack);
    
    text_layer_edit_char_pos = text_layer_create(GRect(3, 168-40, 144, 25));
    text_layer_set_text_alignment(text_layer_edit_char_pos, GTextAlignmentLeft);
    text_layer_set_font(text_layer_edit_char_pos, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_background_color(text_layer_edit_char_pos, GColorClear);
    text_layer_set_text_color(text_layer_edit_char_pos, GColorBlack);
  
    layer_add_child(window_get_root_layer(window_change_entry), text_layer_get_layer(text_layer_edit_left));
    layer_add_child(window_get_root_layer(window_change_entry), text_layer_get_layer(text_layer_edit_right));
    layer_add_child(window_get_root_layer(window_change_entry), text_layer_get_layer(text_layer_edit_top));
    layer_add_child(window_get_root_layer(window_change_entry), text_layer_get_layer(text_layer_edit_bottom));
    layer_add_child(window_get_root_layer(window_change_entry), text_layer_get_layer(text_layer_edit_top_2));
    layer_add_child(window_get_root_layer(window_change_entry), text_layer_get_layer(text_layer_edit_bottom_2));
    layer_add_child(window_get_root_layer(window_change_entry), text_layer_get_layer(text_layer_edit_char_pos));
    
    
    
  } else {
    text_layer_edit = text_layer_create(GRect(0,EDIT_FIELD_Y_POS,144,EDIT_FIELD_HEIGHT));
    text_layer_set_text_alignment(text_layer_edit, GTextAlignmentCenter); // Center the text.
    text_layer_set_font(text_layer_edit, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  }
  
  edit_pos = 0;
  if (!(clear_mode_single_item || clear_mode_all)){
    if (edit_mode_name){
      //snprintf(int2str, sizeof(int2str), "%s", book_list[selected_index].name);
      display_edit_fields();
    } else {
      snprintf(int2str, sizeof(int2str), "Page %d", book_list[selected_index].page_number);
    }
    text_layer_set_text(text_layer_edit, int2str);
  } else {
    if (clear_mode_single_item){
      selected_index = 0;
      snprintf(int2str, sizeof(int2str), "Clear entry %d", selected_index+1);
      text_layer_set_text(text_layer_edit, int2str);
    }
    if (clear_mode_all){
      text_layer_set_text(text_layer_edit, "Clear All ?\nPress Up to continue.");
    }
  }
  layer_add_child(window_get_root_layer(window_change_entry), text_layer_get_layer(text_layer_edit));
  
  window_stack_push(window_change_entry, true); // The back button will dismiss the current window, not close the app. So just press back to go back to the master view.
}




static char next_char(char c, int updown){
  uint8_t ascii = (uint8_t)c;
  char cr; //return character
  if (updown > 0){
    switch (ascii){
      case 32: ascii = 48; break; //' ' -> '0'
      case 57: ascii = 65; break; //'9' -> 'A'
      case 90: ascii = 97; break; //'Z' -> 'a'
      default: ascii++; break;
    }
    if (ascii>122) ascii = 32; //'z' --> ' '
  } else {
    switch (ascii){
      case 48: ascii = 32; break; //'0' -> ' '
      case 65: ascii = 57; break; //'A' -> '9'
      case 97: ascii = 90; break; //'a' -> 'Z'
      default: ascii--; break;
    }
    if (ascii<32) ascii = 122; //' ' --> 'z'
  }
  cr = (char)ascii;
  //check higer order updown recursively:
  if (updown > 0) updown--; else if (updown < 0) updown++;
  if (updown != 0) cr = next_char(cr,updown);
  return cr;
}




static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(text_layer_edit, "Select");
  if (edit_mode_name){
    edit_pos++;
    if (edit_pos >= MAX_NAME_LEN) edit_pos = 0;
    display_edit_fields();
  }
  if (clear_mode_single_item){
    snprintf(book_list[selected_index].name, sizeof(book_list[selected_index].name), "Book %d", selected_index+1);
    book_list[selected_index].page_number = 0;
    save_item(selected_index);
    snprintf(int2str, sizeof(int2str), "Cleared %s", book_list[selected_index].name);
    text_layer_set_text(text_layer_edit, int2str);
  }
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(text_layer_edit, "Up");
  int i;
  if (clear_mode_all){
    //delete everything:
    for (i=0; i<NUM_FIRST_MENU_ITEMS; i++){
      snprintf(book_list[i].name, sizeof(book_list[i].name), "Book %d", i+1);
      book_list[i].page_number = 0;
      save_item(i);
    }
    text_layer_set_text(text_layer_edit, "Cleared All");
    return;
  }
  if (clear_mode_single_item){
    selected_index++;
    if (selected_index >= NUM_FIRST_MENU_ITEMS) selected_index = NUM_FIRST_MENU_ITEMS-1;
    snprintf(int2str, sizeof(int2str), "Clear entry %d", selected_index+1);
    text_layer_set_text(text_layer_edit, int2str);
    return;
  }
  if (edit_mode_name){
    book_list[selected_index].name[edit_pos] = next_char(book_list[selected_index].name[edit_pos], -1);
    //snprintf(int2str, sizeof(int2str), "%s", book_list[selected_index].name);
    display_edit_fields();
  } else {
    book_list[selected_index].page_number += 1;
    snprintf(int2str, sizeof(int2str), "Page %d", book_list[selected_index].page_number);
  }
  text_layer_set_text(text_layer_edit, int2str);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(text_layer_edit, "Down");
  if (clear_mode_single_item){
    selected_index--;
    if (selected_index<0) selected_index = 0;
    snprintf(int2str, sizeof(int2str), "Clear entry %d", selected_index+1);
    text_layer_set_text(text_layer_edit, int2str);
    return;
  }
  if (edit_mode_name){
    book_list[selected_index].name[edit_pos] = next_char(book_list[selected_index].name[edit_pos], +1);
    //snprintf(int2str, sizeof(int2str), "%s", book_list[selected_index].name);
    display_edit_fields();
  } else {
    book_list[selected_index].page_number -= 1;
    snprintf(int2str, sizeof(int2str), "Page %d", book_list[selected_index].page_number);
  }
  text_layer_set_text(text_layer_edit, int2str);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 100, up_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 100, down_click_handler);
}
