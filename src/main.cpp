#include <dpp/dpp.h>
#include <cstdlib>
#include <unordered_map>
#include <vector>
#include <string>
#include <ctime>
#include <chrono>

const std::string BOT_TOKEN = std::getenv("BOT_TOKEN");

std::unordered_multimap<std::string, std::vector<std::string>> tasks;

bool isLeapYear(int year) {
  if (year % 400 == 0) return true;
  if (year % 100 == 0) return false;
  return year % 4 == 0;
}

int daysInMonth(int month, int year) {
  // Array for days in each month (except February)
  static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  // Handle February separately (considering leap years)
  if (month == 2) {
    return isLeapYear(year) ? 29 : 28;
  }

  // Return days from the array for other months
  return days[month - 1];
}

std::vector<std::vector<std::string>> get_tasks_due_tmrw() {
    std::vector<std::vector<std::string>> tasks_due_tmrw;

    time_t now = time(nullptr);
    tm *local_time_tmrw = localtime(&now);
    // Increment day
    local_time_tmrw->tm_mday++;
    // Check if it overflows the current month's days
    if (local_time_tmrw->tm_mday > daysInMonth(local_time_tmrw->tm_mon + 1, local_time_tmrw->tm_year + 1900)) {
      // Overflow, move to next month and potentially next year
      local_time_tmrw->tm_mday = 1;
      local_time_tmrw->tm_mon++;
      if (local_time_tmrw->tm_mon == 12) {
        local_time_tmrw->tm_mon = 0;
        local_time_tmrw->tm_year++;
      }
    }

    for (auto& task : tasks) {
        std::tm task_tm;
        std::istringstream ss(task.first);
        ss >> std::get_time(&task_tm, "%Y-%m-%dT%H:%M:%S");

        if (task_tm.tm_year == local_time_tmrw->tm_year && 
            task_tm.tm_mon == local_time_tmrw->tm_mon && 
            task_tm.tm_mday == local_time_tmrw->tm_mday) {
                tasks_due_tmrw.push_back(task.second);
        }
    }

    return tasks_due_tmrw;
}



int main() {
    dpp::cluster bot(BOT_TOKEN);

    bot.on_log(dpp::utility::cout_logger());

    bot.on_slashcommand([&bot](const dpp::slashcommand_t& event) {

        if (event.command.get_command_name() == "addtask") {
            std::string date = std::get<std::string>(event.get_parameter("date"));
            std::string name = std::get<std::string>(event.get_parameter("name"));
            std::string link = "";
            try {link = std::get<std::string>(event.get_parameter("link"));}
            catch (const std::bad_variant_access& ex){}
            tasks.insert({date, {name, link}});

            event.reply("Added task \"" + name + "\"! I'll remind everyone a day before it's due.");
        }
    });

    bot.on_ready([&bot](const dpp::ready_t& event) {
        if (dpp::run_once<struct register_bot_commands>()) {

            dpp::slashcommand add_task("addtask", "Add a new task reminder", bot.me.id);
            add_task.add_option(dpp::command_option(dpp::co_string, "name", "Name of the task (e.g. learnable: 3.3 Rates of Reaction)", true));
            add_task.add_option(dpp::command_option(dpp::co_string, "date", 
                        "Due date in ISO8601 (2024-06-04T23:59:00 is 11:59pm on 4th June 2024)", true));
            add_task.add_option(dpp::command_option(dpp::co_string, "link", "The link to the task (optional)", false));

            bot.global_command_create(add_task);
        }
    });

    bot.start(true);

    while (true) {
        auto tasks_due_tmrw = get_tasks_due_tmrw();
        std::string message = "Tasks due tomorrow:\n";
        for (auto& task : tasks_due_tmrw) {
            message += "- " + task[0];
            if (task[1].size() > 1) {
                message += ", find it at [link](" + task[1] + ")";
            }
            message += "\n";
        }
        bot.message_create(dpp::message(1055140777421971480, message));
        std::this_thread::sleep_for(std::chrono::hours(24));
    }
}

