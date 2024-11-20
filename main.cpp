#include "RapidXML/rapidxml.hpp"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <chrono>
#include <vector>

const int UPDATE_DELAY = 60;  // How many seconds to wait before updating

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

Event getEvent(rapidxml::xml_node<>* item); // Get the event from an <item> node>
// Callback function to write the data received by libcurl into a string
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
  ((std::string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}
std::string fetchRSSFeed(const std::string& url);
void fetchAndParse(std::string& contentBuffer, rapidxml::xml_document<>& document, const std::string& url);

int main(int argc, char* argv[]) {
  std::string url;
  // Check if URL argument is provided
  if (argc >= 2)
    url = argv[1];
  else
    url = "https://www.monroecounty.gov/incidents911.rss";
  std::string rssContent;
  rapidxml::xml_document<> doc; // Create a document object
  std::vector<Event> events;  // A vector to hold events
  events.reserve(8);  // Reserve space for 8 events to start


  std::chrono::time_point start = std::chrono::steady_clock::now(); // Mark current time, we want to read the file every x seconds
  fetchAndParse(rssContent, doc, url);

  rapidxml::xml_node<> *root = doc.first_node("rss"); // Define root entry point
  rapidxml::xml_node<> *channel = root->first_node("channel"); // Navigate to channel

  // Fill the vector
  for(rapidxml::xml_node<>* item = channel->first_node("item"); item; item = item->next_sibling()) {
    // Push the item to the end of the vector
    events.push_back(getEvent(item));
  }

  // Print the vector
  std::cout << '\n';
  for(auto& event : events) {
    std::cout << event << "\n\n";
  }
  auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::cout << "Last Updated: " << ctime(&time) << std::endl;

  while (true) {
    if((std::chrono::steady_clock::now() - start) > std::chrono::seconds(60)) { // If 60 seconds have passed
      // Get updated data from feed
      fetchAndParse(rssContent, doc, url);
      root = doc.first_node("rss"); // Define root entry point
      channel = root->first_node("channel"); // Navigate to channel
      
      //Clear the vector
      size_t size = events.capacity();
      events.clear();
      events.reserve(size);

      // Fill the vector
      for(rapidxml::xml_node<>* item = channel->first_node("item"); item; item = item->next_sibling()) {
        // Push the item to the end of the vector
        events.push_back(getEvent(item));
      }

      // Print the vector
      std::cout << '\n';
      for(auto& event : events) {
        std::cout << event << "\n\n";
      }
      
      // Reset start time
      start = std::chrono::steady_clock::now();
      auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
      std::cout << "Last Updated: " << ctime(&time) << std::endl;
    }
  }

  return EXIT_SUCCESS;
}

Event getEvent(rapidxml::xml_node<>* item) {
  //Create a temporary event object
    Event event;
  // Extract info into an event item
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

    return event;
}

std::string fetchRSSFeed(const std::string& url) {
  CURL* curl;
  CURLcode res;
  std::string readBuffer;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();

  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // Optional, depending on your SSL setup
    res = curl_easy_perform(curl);

    if(res != CURLE_OK) {
      std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
    }

    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();

  return readBuffer;
}

void fetchAndParse(std::string& contentBuffer, rapidxml::xml_document<>& document, const std::string& url) {
  contentBuffer = fetchRSSFeed(url);
  document.parse<0>(&contentBuffer[0]); // And parse the data from the xml file into it
}
