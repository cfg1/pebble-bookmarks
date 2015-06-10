#include "pebble.h"
#include "set_book.h"

static Window *s_main_window;
static MenuLayer *s_menu_layer;

book_t book_list[NUM_FIRST_MENU_ITEMS];
int edit_mode_name = 0;
int clear_mode_single_item = 0;
int clear_mode_all = 0;




static void load_items(void){
  int i, key;
  for (i=0; i<NUM_FIRST_MENU_ITEMS; i++){
    key = 2*i;
    if (persist_exists(key)) persist_read_string(key, book_list[i].name, sizeof(book_list[i].name));
    key++;
    if (persist_exists(key)) book_list[i].page_number = persist_read_int(key);
  }
}




static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
      return NUM_FIRST_MENU_ITEMS;
    case 1:
      return NUM_SECOND_MENU_ITEMS;
    case 2:
      return NUM_THIRD_MENU_ITEMS;
    default:
      return 0;
  }
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  // Determine which section we're working with
  switch (section_index) {
    case 0:
      // Draw title text in the section header
      menu_cell_basic_header_draw(ctx, cell_layer, "Bookmarks");
      break;
    case 1:
      menu_cell_basic_header_draw(ctx, cell_layer, "Edit Mode");
      break;
    case 2:
      menu_cell_basic_header_draw(ctx, cell_layer, "Clear");
      break;
  }
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  // Determine which section we're going to draw in
  static char int2str[20];
  switch (cell_index->section) {
    case 0:
      // Use the row to specify which item we'll draw
      //cell_index->row
      snprintf(int2str, sizeof(int2str), "Page %d", book_list[cell_index->row].page_number);
      menu_cell_basic_draw(ctx, cell_layer, book_list[cell_index->row].name, int2str, NULL);
      break;
    case 1:
      switch (cell_index->row) {
        case 0:
          // There is title draw for something more simple than a basic menu item
          if (edit_mode_name){
            menu_cell_basic_draw(ctx, cell_layer, "Edit", "Names", NULL);
          } else {
            menu_cell_basic_draw(ctx, cell_layer, "Edit", "Page No.", NULL);
          }
          break;
      }
      break;
    case 2:
      switch(cell_index->row){
        case 0:
          menu_cell_basic_draw(ctx, cell_layer, "Clear", "Entry ...", NULL);
          break;
        case 1:
          menu_cell_basic_draw(ctx, cell_layer, "Clear", "All", NULL);
          break;
      }
  }
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  // Use the row to specify which item will receive the select action
  //cell_index->row
  
  int i;
  
  if ((cell_index->section == 1) && (cell_index->row == 0)){
    edit_mode_name = !edit_mode_name;
  } else if ((cell_index->section == 2) && (cell_index->row == 0)) {
    //delete the selected entry only: 
    clear_mode_all = 0;
    clear_mode_single_item = 1;
    load_settings_page(0);
  } else if ((cell_index->section == 2) && (cell_index->row == 1)) {
    //delete everything:
    clear_mode_all = 1;
    clear_mode_single_item = 0;
    load_settings_page(-1);
  } else if ((cell_index->section == 2) && (cell_index->row == 2)) {
    //delete all names (deactivated):
  } else {
    //section == 0, entry == 0..MAX-1
    clear_mode_all = 0;
    clear_mode_single_item = 0;
    load_settings_page(cell_index->row);
  }
  layer_mark_dirty(menu_layer_get_layer(menu_layer));
}

static void main_window_load(Window *window) {
  // Here we load the bitmap assets
  /*
  s_menu_icons[0] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_ICON_BIG_WATCH);
  s_menu_icons[1] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_ICON_SECTOR_WATCH);
  s_menu_icons[2] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_ICON_BINARY_WATCH);
  */

  // And also load the background
  //s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_BRAINS);
  
  //init list of books:
  int i;
  for (i=0; i<NUM_FIRST_MENU_ITEMS; i++){
    snprintf(book_list[i].name, sizeof(book_list[i].name), "Book %d", i+1);
  }
  //if data was saved, read it:
  load_items(); 
  

  // Now we prepare to initialize the menu layer
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  // Create the menu layer
  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
  });

  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(s_menu_layer, window);

  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void main_window_unload(Window *window) {
  // Destroy the menu layer
  menu_layer_destroy(s_menu_layer);

  // Cleanup the menu icons
  /*
  for (int i = 0; i < NUM_MENU_ICONS; i++) {
    gbitmap_destroy(s_menu_icons[i]);
  }

  gbitmap_destroy(s_background_bitmap);
  */
}

static void init() {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

