#include <iostream>
#include "Channel.h"

int main() {
    
    Channel<string> channel;
    cout << "refs: " << channel.refs << "\n";
    Writer<string> writer(channel);
    cout << "refs: " << channel.refs << "\n";
    Reader<string> reader(writer);
    cout << "refs: " << channel.refs << "\n";
    Reader<string> r2(channel);
    cout << "refs: " << channel.refs << "\n";

    writer.Write("Hello World!");
    writer.Write("Hello Again!");
    cout << reader.Read() << "\n"; 

    channel.RemoveEndPoint(reader);
    cout << "refs: " << channel.refs << "\n";

    cout << reader.Read() << "\n";;
    cout << r2.Read() << "\n";;

    return 0;
}