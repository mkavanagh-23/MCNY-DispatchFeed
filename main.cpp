#include "RapidXML/rapidxml.hpp"
#include "RapidXML/rapidxml_utils.hpp"
#include <cstdlib>
#include <iostream>
#include <string>

struct Event {
  std::string title, link, pubDate, description, guid;
  float latitude, longitude;

  friend std::ostream& operator<<(std::ostream& out, Event& event) {
    out << "Title: " << event.title << '\n';
    out << "URL: " << event.link << '\n';
    out << "Date: " << event.pubDate << '\n';
    out << "Desc: " << event.description << '\n';
    out << "GUID: " << event.guid << '\n';
    out << "GPS: " << '+' << event.latitude << ", -" << event.longitude;
    return out;
  }

};

int main(int argc, char* argv[]) {
  std::vector<Event> events;
  rapidxml::file<> xmlFile("test.xml"); // Load the file into memory
  rapidxml::xml_document<> doc; // Create a document object
  doc.parse<0>(xmlFile.data()); // And parse the data from the xml file into it
  rapidxml::xml_node<> *root = doc.first_node("rss"); // Define root entry point
  rapidxml::xml_node<> *channel = root->first_node("channel"); // Navigate to channel
  
  // Iterate over each <item> node
  for(rapidxml::xml_node<>* item = channel->first_node("item"); item; item = item->next_sibling()) {
    // Create an event to hold the values
    Event event;

    // Extract info into an entry item
    if(rapidxml::xml_node<> *title = item->first_node("title")) {
      event.title = title->value();
    }
    if(rapidxml::xml_node<> *link = item->first_node("link")) {
      event.link = link->value();
    }
    if(rapidxml::xml_node<> *pubDate = item->first_node("pubDate")) {
      event.pubDate = pubDate->value();
    }
    if(rapidxml::xml_node<> *description = item->first_node("description")) {
      event.description = description->value();
    }
    if(rapidxml::xml_node<> *guid = item->first_node("guid")) {
      event.guid = guid->value();
    }
    if(rapidxml::xml_node<> *latitude = item->first_node("geo:lat")) {
      std::string temp = latitude->value();
      event.latitude = std::stof(temp.substr(1));
    }
    if(rapidxml::xml_node<> *longitude = item->first_node("geo:long")) {
      std::string temp = longitude->value();
      event.longitude = std::stof(temp.substr(1));
    }
    
    // Push the item to the end of the vector
    events.push_back(event);
  }

  // And now print each event to confirm we are parsing correctly
  for(auto& event : events) {
    std::cout << event << "\n\n";
  }

  return EXIT_SUCCESS;
}

