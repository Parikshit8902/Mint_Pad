#include <gtkmm.h>
#include <gtksourceviewmm.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/fontchooserdialog.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/notebook.h>
#include <gtkmm/messagedialog.h> 
#include <gtkmm/image.h>       
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <list> 
#include <cstdlib>

// Forward declaration of the class
class IdeWindow;

// A custom widget to hold one editor tab and its associated controls
class EditorTab : public Gtk::ScrolledWindow 
{
  public:
    // Passing a reference to the main window for callbacks
    EditorTab(IdeWindow& parent_window):
      m_parent_window(parent_window),
      m_file_path(""),
      m_language_id("cpp"),
      m_tab_box(Gtk::ORIENTATION_HORIZONTAL)
    {
      m_source_view.set_show_line_numbers(true);
      m_source_view.set_highlight_current_line(true);
      m_source_view.set_auto_indent(true);
      m_source_view.set_smart_home_end(Gsv::SMART_HOME_END_ALWAYS);
      m_source_view.set_insert_spaces_instead_of_tabs(true);
      m_source_view.set_indent_width(4);
      m_source_view.set_name("my-ide-editor");
      add(m_source_view); // Adds the view to the ScrolledWindow

      // Custom tab label widget (Label + Close Button with Icon)
      m_tab_label.set_text("Untitled");
      m_tab_box.pack_start(m_tab_label, Gtk::PACK_SHRINK); 

      // Close button
      Gtk::Image* close_icon = Gtk::manage(new Gtk::Image());
      close_icon->set_from_icon_name("window-close-symbolic", Gtk::ICON_SIZE_SMALL_TOOLBAR); // Standard small 'x' icon
      m_close_button.set_image(*close_icon);
      m_close_button.set_relief(Gtk::RELIEF_NONE); 
      m_close_button.set_tooltip_text("Close Tab");

      m_tab_box.pack_start(m_close_button, Gtk::PACK_SHRINK); 
      m_tab_box.show_all();

      // Connecting the close button signal
      m_close_button.signal_clicked().connect(sigc::mem_fun(*this, &EditorTab::on_close_button_clicked));

      // Connecting buffer modified signal to update tab label
      if(auto buffer = m_source_view.get_source_buffer()) 
      {
      	buffer->signal_modified_changed().connect(sigc::mem_fun(*this, &EditorTab::update_tab_label_widget));
      }
    }

    // Getters
    Gsv::View& get_view() { return m_source_view; }
    std::string get_path() const { return m_file_path; }
    std::string get_language() const { return m_language_id; }
    Gtk::Widget& get_tab_widget() { return m_tab_box; }

    std::string get_base_filename() const 
    {
      if (m_file_path.empty()) 
      {
        return "Untitled";
      }
      size_t pos = m_file_path.find_last_of("/");
      return (pos == std::string::npos) ? m_file_path : m_file_path.substr(pos + 1);
    }

    bool is_modified() const 
    {
      if(auto buffer = m_source_view.get_source_buffer()) 
      {
        return buffer->get_modified();
      }
      return false;
    }

    // Setters
    void set_path(const std::string& path) 
    {
      m_file_path = path;
      update_tab_label_widget();
    }

    void set_language(const std::string& lang_id) 
    {
      m_language_id = lang_id;
      auto lang_manager = Gsv::LanguageManager::get_default();
      auto lang = lang_manager->get_language(m_language_id);
      if(auto buffer = m_source_view.get_source_buffer()) 
      {
        buffer->set_language(lang);
        buffer->set_modified(buffer->get_modified()); // Treats language change similar to loading new content regarding modification
      }
      update_tab_label_widget(); // Ensure label updates
    }

    void set_font(const std::string& font_desc) 
    {
      m_source_view.override_font(Pango::FontDescription(font_desc));
    }
    
    // File Operations
    bool load_file(const std::string& path) 
    {
      std::ifstream infile(path);
      if (infile.is_open()) 
      {
        std::stringstream sstr;
        sstr << infile.rdbuf();
        if(auto buffer = m_source_view.get_source_buffer()) 
        {
          buffer->set_text(sstr.str());
          buffer->set_modified(false); // File loaded, not modified
        }
        infile.close();
        set_path(path); 

        // Setting language based on file extension
        if (path.length() > 2 && path.substr(path.length() - 2) == ".c") 
        {
          set_language("c");
        } 
        else if (path.length() > 3 && path.substr(path.length() - 3) == ".py") 
        {
          set_language("python");
        } 
        else 
        { 
          set_language("cpp"); //cpp by default
        }
        return true;
      }
      return false;
    }

    bool save_file(const std::string& path) 
    {
      if(auto buffer = m_source_view.get_source_buffer()) 
      {
        std::string code = buffer->get_text();
        std::ofstream outfile(path);
        if (outfile.is_open()) 
        {
          outfile << code;
          outfile.close();
          set_path(path); // Update path after successful save
	  buffer->set_modified(false); // Mark as saved
          update_tab_label_widget();
          return true;
        }
      }
      return false;
    }

    // Signal Handlers
    void on_close_button_clicked(); // Implementation requires IdeWindow, defined later

    // Updates the Gtk::Label in the tab widget
    void update_tab_label_widget() 
    {
      std::string label_text = get_base_filename();
      if (is_modified()) 
      {
        label_text += "*"; // Adds * for modified files
      }
      m_tab_label.set_text(label_text);
    }

  protected:
    IdeWindow& m_parent_window; // References to parent
    Gsv::View m_source_view;
    std::string m_file_path;
    std::string m_language_id;

    // Widgets for the custom tab label
    Gtk::Box m_tab_box;
    Gtk::Label m_tab_label;
    Gtk::Button m_close_button;
};

// Main application window
class IdeWindow : public Gtk::Window 
{
  public:
    IdeWindow();
    void close_tab(EditorTab* tab_to_close); // Close tab

  protected:
    // Override the window delete event for save prompts
    bool on_delete_event(GdkEventAny* event) override;

    // Signal handlers
    void on_run_button_clicked();
    void on_language_changed();
    void on_new_clicked();
    void on_open_clicked();
    void on_save_clicked();
    void on_save_as_clicked();
    void on_exit_clicked();
    void on_quit_button_clicked(); 
    void on_dark_theme_toggled();
    void on_font_clicked();
    void on_cursor_position_changed(const Gtk::TextBuffer::iterator& iter, const Glib::RefPtr<Gtk::TextBuffer::Mark>& mark);
    void on_tab_changed(Gtk::Widget* page, guint page_num);

    // Helper functions
    EditorTab* get_current_tab();
    void create_new_tab(const std::string& file_path = "");
    void update_title();
    void update_statusbar();
    bool save_current_tab_if_needed(EditorTab* tab); // Helper for save logic
    
    // Child Widgets
    Gtk::HeaderBar m_header_bar;
    Gtk::Button m_run_button;
    Gtk::ComboBoxText m_language_combo;
    Gtk::MenuButton m_file_menu_button;
    Gtk::Popover m_file_popover;
    Gtk::Box m_file_menu_box;
    Gtk::ModelButton m_new_button;
    Gtk::ModelButton m_open_button;
    Gtk::ModelButton m_save_button;
    Gtk::ModelButton m_save_as_button;
    Gtk::ModelButton m_dark_theme_button;
    Gtk::ModelButton m_font_button;
    Gtk::ModelButton m_exit_button;
    Gtk::Button m_quit_button;
    Gtk::Box m_main_box;

    Gtk::Notebook m_notebook;
    Gtk::Statusbar m_statusbar;

    // State variables
    std::string m_font_desc;
    bool m_dark_theme_active;
    Glib::RefPtr<Gtk::CssProvider> m_css_provider;
};

// IdeWindow Implementation
IdeWindow::IdeWindow() :
  m_main_box(Gtk::ORIENTATION_VERTICAL),
  m_run_button("Run"),
  m_file_menu_box(Gtk::ORIENTATION_VERTICAL),
  m_dark_theme_active(false)
{
  set_default_size(800, 600);
  set_titlebar(m_header_bar);

  // "File" Menu Setup
  m_file_menu_button.set_label("File");
  m_new_button.set_label("New");
  m_open_button.set_label("Open...");
  m_save_button.set_label("Save");
  m_save_as_button.set_label("Save As...");
  m_dark_theme_button.set_label("Toggle Dark Theme");
  m_font_button.set_label("Preferences...");
  m_exit_button.set_label("Exit");

  m_file_menu_button.set_popover(m_file_popover);
  m_file_popover.add(m_file_menu_box);
  m_file_menu_box.pack_start(m_new_button, true, true, 0);
  m_file_menu_box.pack_start(m_open_button, true, true, 0);
  m_file_menu_box.pack_start(m_save_button, true, true, 0);
  m_file_menu_box.pack_start(m_save_as_button, true, true, 0);
  m_file_menu_box.pack_start(m_dark_theme_button, true, true, 0);
  m_file_menu_box.pack_start(m_font_button, true, true, 0);
  m_file_menu_box.pack_start(m_exit_button, true, true, 0);

  m_new_button.signal_clicked().connect(sigc::mem_fun(*this, &IdeWindow::on_new_clicked));
  m_open_button.signal_clicked().connect(sigc::mem_fun(*this, &IdeWindow::on_open_clicked));
  m_save_button.signal_clicked().connect(sigc::mem_fun(*this, &IdeWindow::on_save_clicked));
  m_save_as_button.signal_clicked().connect(sigc::mem_fun(*this, &IdeWindow::on_save_as_clicked));
  m_dark_theme_button.signal_clicked().connect(sigc::mem_fun(*this, &IdeWindow::on_dark_theme_toggled));
  m_font_button.signal_clicked().connect(sigc::mem_fun(*this, &IdeWindow::on_font_clicked));
  m_exit_button.signal_clicked().connect(sigc::mem_fun(*this, &IdeWindow::on_exit_clicked));
  m_file_menu_box.show_all();

  // Language Selector
  m_language_combo.append("cpp", "C++");
  m_language_combo.append("c", "C");
  m_language_combo.append("python", "Python");
  m_language_combo.signal_changed().connect(sigc::mem_fun(*this, &IdeWindow::on_language_changed));

  // Quit Button
  Gtk::Image* quit_icon = Gtk::manage(new Gtk::Image());
  quit_icon->set_from_icon_name("window-close-symbolic", Gtk::ICON_SIZE_BUTTON); 
  m_quit_button.set_image(*quit_icon);
  m_quit_button.set_relief(Gtk::RELIEF_NONE);
  m_quit_button.set_tooltip_text("Quit Application");

  // HeaderBar Layout
  m_header_bar.pack_start(m_file_menu_button);
  m_header_bar.pack_start(m_language_combo);
  m_header_bar.pack_start(m_run_button); 
  m_header_bar.pack_end(m_quit_button); 
  m_run_button.signal_clicked().connect(sigc::mem_fun(*this, &IdeWindow::on_run_button_clicked));
  m_quit_button.signal_clicked().connect(sigc::mem_fun(*this, &IdeWindow::on_quit_button_clicked)); // Connecting quit

  add(m_main_box);

  // Notebook (Tabs) Setup 
  m_notebook.set_scrollable(true);
  m_notebook.signal_switch_page().connect(sigc::mem_fun(*this, &IdeWindow::on_tab_changed));

  m_main_box.pack_start(m_notebook, true, true, 0);
  m_main_box.pack_start(m_statusbar, false, false, 0);

  m_css_provider = Gtk::CssProvider::create();

  create_new_tab(); // Creates the first tab

  show_all_children();
}

// Overrides delete event (called when window close button is pressed or hide() is called)
bool IdeWindow::on_delete_event(GdkEventAny* event) 
{
  bool has_unsaved = false;
  std::list<EditorTab*> unsaved_tabs; // Store pointers to unsaved tabs

  for (int i = 0; i < m_notebook.get_n_pages(); ++i) 
  {
    auto tab = dynamic_cast<EditorTab*>(m_notebook.get_nth_page(i));
    if (tab && tab->is_modified()) 
    {
      has_unsaved = true;
      unsaved_tabs.push_back(tab);
    }
  }

  if (has_unsaved) 
  {
    Gtk::MessageDialog dialog(*this, "You have unsaved changes.", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE);
    // Using stock IDs for standard button text and order
    dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_YES); // Label: "Save"
    dialog.add_button("_Discard Changes", Gtk::RESPONSE_NO); // Label: "Discard Changes"
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL); // Label: "Cancel"
    dialog.set_secondary_text("Do you want to save the changes before quitting?");

    int result = dialog.run();

    if (result == Gtk::RESPONSE_YES) 
    { 
      bool all_saved = true; // Corresponds to "Save"
      for (EditorTab* tab : unsaved_tabs) 
      {
        if (!save_current_tab_if_needed(tab)) // Try to save each unsaved tab
        {
          all_saved = false;
          int page_num = m_notebook.page_num(*tab);
          if (page_num >= 0) 
          {
            m_notebook.set_current_page(page_num);
          }
          break; // Stops saving if one fails/cancels
        }
      }
      if (!all_saved) 
      {
        return true; // Prevents closing if save failed/cancelled
      }
      // If all saved successfully
    } 
    else if (result == Gtk::RESPONSE_CANCEL) 
    {
      return true; // Prevents closing
    }
    // If "Discard Changes" (RESPONSE_NO)
  }

  return false; // Allow closing (either no unsaved files or user chooses Discard/Save All succeeded)
}


// Get current tab
EditorTab* IdeWindow::get_current_tab() 
{
  if (m_notebook.get_n_pages() == 0) return nullptr;
  int current_page_index = m_notebook.get_current_page();
  if (current_page_index < 0) return nullptr;
  auto current_page = m_notebook.get_nth_page(current_page_index);
  return dynamic_cast<EditorTab*>(current_page);
}

// Creating new tab
void IdeWindow::create_new_tab(const std::string& file_path) 
{
  EditorTab* tab = Gtk::manage(new EditorTab(*this));

  if(auto buffer = tab->get_view().get_source_buffer()) 
  {
    buffer->signal_mark_set().connect(sigc::mem_fun(*this, &IdeWindow::on_cursor_position_changed));
  }

  if (!m_font_desc.empty()) 
  {
    tab->set_font(m_font_desc);
  }

  std::string initial_language = "cpp";
  if (!file_path.empty()) 
  {
    if (!tab->load_file(file_path)) 
    {
      Gtk::MessageDialog err_dialog(*this, "Error opening file: " + file_path, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK); // Error loading file (e.g., doesn't exist)
      err_dialog.run();
      // Gtk::manage will delete the tab when it goes out of scope here if load fails
      // No need to explicitly delete managed widget
      return;
    }
    initial_language = tab->get_language();
  } 
  else 
  {
    tab->set_language(initial_language);
  }

  m_notebook.append_page(*tab, tab->get_tab_widget());
  m_notebook.show_all();
  m_notebook.set_current_page(-1); // Switching focus to the new tab
}

// Handles "New"
void IdeWindow::on_new_clicked() 
{
  create_new_tab();
}

// Handles "Open"
void IdeWindow::on_open_clicked() 
{
  Gtk::FileChooserDialog dialog("Please choose a file", Gtk::FILE_CHOOSER_ACTION_OPEN);
  dialog.set_transient_for(*this);
  dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL); 
  dialog.add_button("_Open", Gtk::RESPONSE_ACCEPT);

  int result = dialog.run();

  if (result == Gtk::RESPONSE_ACCEPT) 
  {
    create_new_tab(dialog.get_filename());
  }
}

// Handles "Save"
void IdeWindow::on_save_clicked() 
{
  EditorTab* tab = get_current_tab();
  save_current_tab_if_needed(tab); // Use the helper
}

// Handles "Save As" logic, returns true if save succeeded or wasn't needed, false if cancelled/failed
bool IdeWindow::save_current_tab_if_needed(EditorTab* tab) 
{
  if (!tab) return false;

  if (tab->get_path().empty() || tab->is_modified()) // Only prompt/save if the file is untitled or modified
  {
    if (tab->get_path().empty()) 
    {
      Gtk::FileChooserDialog dialog("Save File As", Gtk::FILE_CHOOSER_ACTION_SAVE);
      dialog.set_transient_for(*this);
      dialog.set_do_overwrite_confirmation(true); // Warns if overwriting
      dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
      dialog.add_button("_Save", Gtk::RESPONSE_ACCEPT);

      std::string default_name = "untitled";
      if (tab->get_language() == "cpp") default_name += ".cpp";
      else if (tab->get_language() == "c") default_name += ".c";
      else if (tab->get_language() == "python") default_name += ".py";
      dialog.set_current_name(default_name);

      int result = dialog.run();
      if (result == Gtk::RESPONSE_ACCEPT) 
      {
        std::string filename = dialog.get_filename();
        if (tab->save_file(filename)) 
        {
          update_title(); // Updates title after successful save as
          return true;
        } 
        else 
        {
          Gtk::MessageDialog err_dialog(*this, "Error saving file.", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
          err_dialog.run();
          return false; // Save failed
        }
      } 
      else 
      {
        return false; // User cancelled Save As
      }
    } 
    else 
    {
      return tab->save_file(tab->get_path()); // Already has a path, just save
    }
  }
  return true; // Not modified, "save succeeded"
}


// "Save As" menu item just forces the "Save As" part of the helper
void IdeWindow::on_save_as_clicked() 
{
  EditorTab* tab = get_current_tab();
  if (!tab) return;

  std::string original_path = tab->get_path();
  tab->set_path(""); // Temporarily clear path to force "Save As" dialog
  bool saved = save_current_tab_if_needed(tab);
  if (!saved && !original_path.empty()) // Restore original path only if save was cancelled and it had an original path
  {
    tab->set_path(original_path);
    tab->update_tab_label_widget();
  }
  // If saved is true, save_current_tab_if_needed already updated the path and label
}


// Public method to actually close a tab, handles prompts
void IdeWindow::close_tab(EditorTab* tab_to_close) 
{
  if (!tab_to_close) return;

  bool do_close = true; // Assume we will close unless cancelled

  if (tab_to_close->is_modified()) 
  {
    Gtk::MessageDialog dialog(*this, "Save changes before closing?", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE);
    dialog.set_secondary_text("\"" + tab_to_close->get_base_filename() + "*\" has unsaved changes.");
    dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_YES);
    dialog.add_button("_Don't Save", Gtk::RESPONSE_NO); 
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

    int result = dialog.run();

    if (result == Gtk::RESPONSE_YES) 
    { 
      if (!save_current_tab_if_needed(tab_to_close)) // Save
      {
        do_close = false; // Save failed or cancelled
      }
    } 
    else if (result == Gtk::RESPONSE_CANCEL) 
    {
      do_close = false; // User cancelled
    }
    // If "Don't Save" (RESPONSE_NO), do_close remains true
  }

  if (do_close) 
  {
    int page_num = m_notebook.page_num(*tab_to_close);
    if (page_num >= 0) 
    {
      m_notebook.remove_page(page_num);
      // Gtk::manage handles deletion automatically

      if (m_notebook.get_n_pages() == 0) 
      {
        create_new_tab(); // Ensures at least one tab remains
      } 
      else 
      {
         on_tab_changed(nullptr, m_notebook.get_current_page()); // Trigger updates for the potentially new current tab
      }
    }
  }
}

// Other Handlers

void IdeWindow::on_font_clicked() 
{
  Gtk::FontChooserDialog dialog("Choose Font", *this);
  if (!m_font_desc.empty()) 
  {
    dialog.set_font(m_font_desc);
  }

  int result = dialog.run();

  if (result == Gtk::RESPONSE_OK) 
  {
    m_font_desc = dialog.get_font();
    for (int i = 0; i < m_notebook.get_n_pages(); ++i) 
    {
      auto tab = dynamic_cast<EditorTab*>(m_notebook.get_nth_page(i));
      if (tab) tab->set_font(m_font_desc);
    }
  }
}

void IdeWindow::on_cursor_position_changed(const Gtk::TextBuffer::iterator& iter, const Glib::RefPtr<Gtk::TextBuffer::Mark>& mark) 
{
  EditorTab* current_tab = get_current_tab(); // Checks if the mark is the insert cursor and if the currently focused tab is the one emitting the signal
  if (current_tab && mark->get_name() == "insert" && mark->get_buffer() == current_tab->get_view().get_source_buffer()) 
  {
    update_statusbar();
    update_title(); // Updates title to potentially show/hide * (asterisk)
  }
}

void IdeWindow::on_tab_changed(Gtk::Widget* page, guint page_num) 
{
  EditorTab* tab = get_current_tab();
  if (tab) 
  {
    update_statusbar();
    update_title();
    m_language_combo.set_active_id(tab->get_language());
  } 
  else 
  {
    update_title(); // Updates to default title
    update_statusbar(); // Clears status bar
  }
}

void IdeWindow::update_statusbar() 
{
  EditorTab* tab = get_current_tab();
  if (tab) 
  {
    auto buffer = tab->get_view().get_source_buffer();
    if(!buffer) return;
    auto iter = buffer->get_iter_at_mark(buffer->get_insert());
    int line = iter.get_line() + 1;
    int col = iter.get_line_offset() + 1;
    std::string status = "Line: " + std::to_string(line) + ", Col: " + std::to_string(col);
    m_statusbar.pop(); // Removes previous message context id 0
    m_statusbar.push(status); // Pushes new message context id 0
  } 
  else 
  {
    m_statusbar.pop(); // Clears status bar if no tab
  }
}

void IdeWindow::update_title() 
{
  std::string title_base = "Mint_Pad"; // It's your own, personal editor, name it whatever you want. I went with Mint_Pad since it is developed in Linux Mint.
  EditorTab* tab = get_current_tab();
  if (tab) 
  {
    set_title(tab->get_base_filename() + (tab->is_modified() ? "*" : "") + " - " + title_base);
  } 
  else 
  {
    set_title(title_base); // Default title if no tabs
  }
}

void IdeWindow::on_language_changed() 
{
  EditorTab* tab = get_current_tab();
  if (tab) 
  {
    tab->set_language(m_language_combo.get_active_id());
    tab->update_tab_label_widget(); // Update tab title if needed (like Untitled -> Untitled*)
    update_title(); // Also update main window title if needed
  }
}

void IdeWindow::on_run_button_clicked() 
{
  EditorTab* tab = get_current_tab();
  if (!tab) return;

  if (tab->is_modified() && !tab->get_path().empty()) // Saves current file if needed before running 
  {
    tab->save_file(tab->get_path());
  }

  std::string current_language = tab->get_language();
  std::string source_filename_base; // Just the name part
  std::string exec_filename_base = "temp_ide_exec"; // Base name for executable
  std::string build_command;
  std::string run_command;
  std::string cleanup_command;

  // Define filenames using /tmp directory
  std::string temp_dir = "/tmp/"; // Standard Linux temporary directory
  std::string source_filepath;
  std::string exec_filepath = temp_dir + exec_filename_base;

  if (current_language == "cpp") 
  {
    source_filename_base = "temp_run.cpp";
    source_filepath = temp_dir + source_filename_base;
    build_command = "g++ " + source_filepath + " -o " + exec_filepath;
    run_command = exec_filepath; // Run the executable from /tmp
    cleanup_command = "rm -f " + source_filepath + " " + exec_filepath; // Removes both upon closing the terminal
  } 
  else if (current_language == "c") 
  {
    source_filename_base = "temp_run.c";
    source_filepath = temp_dir + source_filename_base;
    build_command = "gcc " + source_filepath + " -o " + exec_filepath;
    run_command = exec_filepath;
    cleanup_command = "rm -f " + source_filepath + " " + exec_filepath;
  } 
  else if (current_language == "python") 
  {
    source_filename_base = "temp_run.py";
    source_filepath = temp_dir + source_filename_base;
    build_command = ""; // No build step for Python
    run_command = "python3 " + source_filepath;
    cleanup_command = "rm -f " + source_filepath; // Only remove source upon closing the terminal
  } 
  else 
  {
    return; // Unknown language
  }

  if(auto buffer = tab->get_view().get_source_buffer()) // Save the current tab's buffer to the temporary file
  {
    std::string code = buffer->get_text();
    std::ofstream outfile(source_filepath); // Save to /tmp/temp_run.*
    if (outfile.is_open()) 
    {
      outfile << code;
      outfile.close();
    } 
    else 
    {
      Gtk::MessageDialog err_dialog(*this, "Error: Could not write temporary file in /tmp.", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
      err_dialog.run();
      return;
    }
  } 
  else 
  {
    return; // Should not happen
  }

  // Construct the full command for gnome-terminal
  std::string full_command_in_terminal;
  std::string pause_part = "echo; read -p 'Press Enter to close...'"; // Added echo for spacing

  if (!build_command.empty()) 
  { // For C/C++
    // Compile && Run || Error ; Cleanup ; Pause
    full_command_in_terminal = build_command + " && " + run_command + " || echo '--- COMPILATION FAILED ---'; " + cleanup_command + "; " + pause_part;
  } 
  else 
  { // For Python
    // Run ; Cleanup ; Pause
    full_command_in_terminal = run_command + "; " + cleanup_command + "; " + pause_part;
  }
  
  // Escapes double quotes inside the command string for the shell
  size_t pos = 0; 
  while ((pos = full_command_in_terminal.find('"', pos)) != std::string::npos) 
  {
    full_command_in_terminal.replace(pos, 1, "\\\"");
    pos += 2; // Move past the replaced sequence
  }

  std::string terminal_command = "gnome-terminal -- bash -c \"" + full_command_in_terminal + "\"";

  system((terminal_command + " &").c_str()); // Run the command in the background
}

// Exit button and Quit button both just trigger the window close
void IdeWindow::on_exit_clicked() 
{
  hide(); // This will trigger on_delete_event
}

void IdeWindow::on_quit_button_clicked() 
{
  hide(); // This will trigger on_delete_event
}

void IdeWindow::on_dark_theme_toggled() 
{
  auto screen = Gdk::Screen::get_default();
  if (m_dark_theme_active) 
  {
    Gtk::StyleContext::remove_provider_for_screen(screen, m_css_provider);
    m_dark_theme_active = false;
  } 
  else 
  {
    try 
    {
      m_css_provider->load_from_path("dark.css");
      Gtk::StyleContext::add_provider_for_screen(screen, m_css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
      m_dark_theme_active = true;
    } 
    catch (const Gtk::CssProviderError& ex) 
    {
      std::cerr << "CssProviderError: " << ex.what() << std::endl;
    } 
    catch (const Glib::FileError& ex) 
    {
      std::cerr << "FileError: " << ex.what() << ". Make sure 'dark.css' is in the same directory." << std::endl;
    }
  }
}

// EditorTab::on_close_button_clicked Implementation 
// Needs to be defined after IdeWindow is fully defined
void EditorTab::on_close_button_clicked() 
{
  m_parent_window.close_tab(this);
}

// Main Function 
int main(int argc, char* argv[]) 
{
  auto app = Gtk::Application::create(argc, argv, "org.gtkmm.examples.ide");
  Gsv::init();
  IdeWindow window;
  return app->run(window);
}
