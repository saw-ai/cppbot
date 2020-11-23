#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/SQLite/Connector.h>


#include <jsoncpp/json/json.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/writer.h>

#include <stdlib.h>
#include <sstream>
#include <stdexcept>

#include <iostream>
#include <string>
#include <fstream>


using namespace Poco::Net;
using namespace Poco;

typedef int32_t STATUS;
#define SUCCESS 0
#define FAIL    1


struct Message {
    int64_t user_id;
    int64_t chat_id;
    int64_t update_id;
    std::string text;

    Message(Json::Value msg) {

        try {
            user_id = msg["message"]["from"]["id"].asInt64();
            update_id = msg["update_id"].asInt64();
            text = msg["message"]["text"].asString();
            chat_id = msg["message"]["chat"]["id"].asInt64();

        } catch (const std::exception& e){
            std::cout << e.what() << std::endl;
        }


    }
};





void PrintJSONValue( const Json::Value &val )
{
    if( val.isString() ) {
        printf( "string(%s)", val.asString().c_str() );
    } else if( val.isBool() ) {
        printf( "bool(%d)", val.asBool() );
    } else if( val.isInt() ) {
        printf( "int(%d)", val.asInt() );
    } else if( val.isUInt() ) {
        printf( "uint(%u)", val.asUInt() );
    } else if( val.isDouble() ) {
        printf( "double(%f)", val.asDouble() );
    }
    else
    {
        printf( "unknown type=[%d]", val.type() );
    }
}

bool PrintJSONTree( const Json::Value &root, unsigned short depth /* = 0 */)
{
    depth += 1;
    printf( " {type=[%d], size=%d}", root.type(), root.size() );

    if( root.size() > 0 ) {
        printf("\n");
        for( Json::Value::const_iterator itr = root.begin() ; itr != root.end() ; itr++ ) {
            // Print depth.
            for( int tab = 0 ; tab < depth; tab++) {
                printf("--");
            }
            printf(" subvalue(");
            PrintJSONValue(itr.key());
            printf(") -");
            PrintJSONTree( *itr, depth);
        }
        return true;
    } else {
        printf(" ");
        PrintJSONValue(root);
        printf( "\n" );
    }
    return true;
}




std::string ParseSentence(std::string sentence) {

    Json::Value value;

    value["text"] = sentence;
    value["model"] = "en_core_web_sm";
    value["collapse_phrases"] = true;
    value["collapse_punctuation"] = true;

    Json::StreamWriterBuilder builder;
    builder.settings_["indentation"] = "";
    std::string str_value = Json::writeString(builder, value);


    std::cout << str_value << std::endl;

    URI uri("https://api.explosion.ai/displacy/dep/");
    HTTPSClientSession session(uri.getHost(), uri.getPort());

    std::string path(uri.getPathAndQuery());

    HTTPRequest req(HTTPRequest::HTTP_POST, path, HTTPMessage::HTTP_1_1);

    HTTPResponse response;

    req.setContentType("application/json");
    req.setContentLength(str_value.length());

    session.sendRequest(req) << str_value;


    HTTPResponse res;
    std::cout << res.getStatus() << " " << res.getReason() << std::endl;

    std::istream &is = session.receiveResponse(res);

    Json::CharReaderBuilder builder2;
    Json::Value root;
    std::string errs;

    bool ok = Json::parseFromStream(builder2, is, &root, &errs);

    //PrintJSONTree(root, 0);

    std::string answer = "";
    for (auto it = root["words"].begin(); it != root["words"].end(); ++it) {
        answer += (*it)["text"].asString() + "(" + (*it)["tag"].asString() + ") ";
    }


    return answer;

}


class Bot {
    int64_t offset;
    std::string base_uri = "https://api.telegram.org/bot507869820:AAG4kmBPK6He6UTd1UONzO6cDux6hynKBwA/";

public:
    Bot() {
        try {
            load_offset();
        } catch (const std::exception& e) {
            offset = 0;
            save_offset();
        }
    }

    void save_offset() {
        std::ofstream fout("offset");
        fout << offset;
        fout.close();
    }

    void load_offset() {
        std::ifstream fin("offset");
        fin >> offset;
        fin.close();
    }

    std::vector<Message> get_updates() {

        std::vector<Message> messages;

        URI uri(base_uri + "getUpdates?offset=" + std::to_string(offset + 1));
        HTTPSClientSession session(uri.getHost(), uri.getPort());

        std::string path(uri.getPathAndQuery());

        HTTPRequest req(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
        session.sendRequest(req);

        HTTPResponse res;
        std::cout << res.getStatus() << " " << res.getReason() << std::endl;

        std::istream &is = session.receiveResponse(res);

        Json::CharReaderBuilder builder;
        Json::Value root;
        std::string errs;

        bool ok = Json::parseFromStream(builder, is, &root, &errs);



        if (root["ok"].asBool()) {

            for (auto it = root["result"].begin(); it != root["result"].end(); ++it) {
                messages.push_back(Message(*it));
            }

        }

        return messages;


    }

    void send_message(int64_t chat_id, std::string text) {
        URI uri(base_uri + "sendMessage?chat_id=" + std::to_string(chat_id)
                + "&text=" + text);
        HTTPSClientSession session(uri.getHost(), uri.getPort());

        std::string path(uri.getPathAndQuery());

        HTTPRequest req(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
        session.sendRequest(req);

        HTTPResponse res;
        std::cout << res.getStatus() << " " << res.getReason() << std::endl;

        std::istream &is = session.receiveResponse(res);

        Json::CharReaderBuilder builder;
        Json::Value root;
        std::string errs;

        bool ok = Json::parseFromStream(builder, is, &root, &errs);

        PrintJSONTree(root, 0);

    }


    void process_messages(const std::vector<Message> messages) {

        for (auto message : messages) {

            offset = message.update_id;
            save_offset();

            if (message.text == "/start") {
                send_message(message.chat_id, "Use  /parse This is a simple sentence  (Powered by spacy.io)");
            } else if (message.text == "/random") {
                send_message(message.chat_id, std::to_string(std::rand()));
            } else if (message.text == "/weather") {
                send_message(message.chat_id, "Winter Is Coming");
            } else if (message.text == "/styleguide") {
                send_message(message.chat_id, "Funny joke");
            } else if (message.text == "/stop") {
                exit(0);
            } else if (message.text == "/crash") {
                abort();
            } else if (message.text.substr(0, 6) == "/parse") {
                auto parsed = ParseSentence(message.text.substr(7));
                send_message(message.chat_id, parsed);
            }

        }
    }

};


int main()
{
    Bot bot;

    while (true) {

        auto messages = bot.get_updates();
        bot.process_messages(messages);

        sleep(5);
    }

    return 0;
}
