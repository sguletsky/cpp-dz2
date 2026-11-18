#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

struct InventoryItem {
  int id;
  std::string name;

  void print() const { std::cout << id << ' ' << name << '\n'; }
};

class Fleet {
  std::vector<std::shared_ptr<InventoryItem>> items_;
  std::weak_ptr<InventoryItem> focus_;

  [[nodiscard]] auto find(int id) const {
    return std::find_if(items_.cbegin(), items_.cend(),
                        [id](const auto& item) { return item->id == id; });
  }

 public:
  void add(int id, const std::string& name, bool passedInspection) {
    auto item = std::make_unique<InventoryItem>(InventoryItem{id, name});

    if (!passedInspection) {
      throw std::runtime_error("inspection failed");
    }
    if (find(id) != items_.cend()) {
      throw std::runtime_error("duplicate id");
    }

    items_.emplace_back(std::move(item));
  }

  void remove(int id) {
    const auto position = find(id);
    if (position == items_.cend()) {
      throw std::runtime_error("id not found");
    }
    items_.erase(position);
  }

  void focus(int id) {
    const auto position = find(id);
    if (position == items_.cend()) {
      throw std::runtime_error("id not found");
    }
    focus_ = *position;
  }

  void showFocus() const {
    const auto item = focus_.lock();
    if (item) {
      item->print();
    } else {
      std::cout << "expired\n";
    }
  }

  void list() const {
    if (items_.empty()) {
      std::cout << "empty\n";
    }
    for (const auto& item : items_) {
      item->print();
    }
  }
};

int readId(std::istream& input) {
  std::string token;
  input >> token;

  std::istringstream number(token);
  int id = 0;
  if (!(number >> id) || id <= 0 || !number.eof() || token.front() == '+') {
    throw std::runtime_error("expected a positive integer id");
  }
  return id;
}

void requireEnd(std::istream& input) {
  std::string extra;
  if (input >> extra) {
    throw std::runtime_error("unexpected arguments");
  }
}

int main() {
  Fleet fleet;

  for (std::string line; std::getline(std::cin, line);) {
    std::istringstream input(line);
    std::string command;
    if (!(input >> command)) {
      continue;
    }

    try {
      if (command == "add") {
        const int id = readId(input);
        std::string condition;
        std::string name;
        input >> condition;
        std::getline(input >> std::ws, name);

        if ((condition != "ok" && condition != "fail") || name.empty()) {
          throw std::runtime_error("usage: add <id> <ok|fail> <name>");
        }

        fleet.add(id, name, condition == "ok");
        std::cout << "added\n";
      } else if (command == "remove" || command == "focus") {
        const int id = readId(input);
        requireEnd(input);

        if (command == "remove") {
          fleet.remove(id);
          std::cout << "removed\n";
        } else {
          fleet.focus(id);
          std::cout << "focused\n";
        }
      } else if (command == "show_focus") {
        requireEnd(input);
        fleet.showFocus();
      } else if (command == "list") {
        requireEnd(input);
        fleet.list();
      } else if (command == "quit") {
        requireEnd(input);
        break;
      } else {
        throw std::runtime_error("unknown command");
      }
    } catch (const std::runtime_error& error) {
      std::cout << "error: " << error.what() << '\n';
    }
  }
}