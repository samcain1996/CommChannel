#include <queue>
#include <mutex>
#include <set>
#include <vector>
#include <algorithm>
#include <unordered_map>

using namespace std;

template <typename T>
class EndPoint;

template <typename T>
class Reader;

template <typename T>
struct Channel {

//private:
    int refs = 0;

public:
    mutex mutex;
    queue<T> queue;

    bool AddEndPoint(EndPoint<T>& endpoint) {
        if (endpoint.ConnectedToChannel() || 
            endpoint.pChannel == this) { return false; }

        refs++;
        endpoint.pChannel = this;
        return true;
    }
    bool AddEndPoint(EndPoint<T>* endpoint) {
        return AddEndPoint(*endpoint);
    }
    bool RemoveEndPoint(EndPoint<T>& endpoint) {
        if (!endpoint.ConnectedToChannel() || 
            endpoint.pChannel != this) { return false; }

        refs--;
        endpoint.pChannel = nullptr;
        return true;
    }
    bool RemoveEndPoint(EndPoint<T>* endpoint) {
       return RemoveEndPoint(*endpoint);
    }
};

template <typename T>
class EndPoint {

    friend class Channel<T>;

protected:

    Channel<T>* pChannel = nullptr;

    EndPoint() {}
    EndPoint(const EndPoint<T>& other) { other.pChannel->AddEndPoint(this); }
    EndPoint(EndPoint<T>&& other) noexcept : pChannel(move(other.pChannel)) {}
    EndPoint(Channel<T>& channel) { channel.AddEndPoint(this); }

    ~EndPoint() {}

    EndPoint<T>& operator=(const EndPoint<T>& other) {
        pChannel = other.pChannel;
        return *this;
    }

    EndPoint<T>& operator=(EndPoint<T>&& other) {
        pChannel = move(other.pChannel);
        return *this;
    }

public:

    bool ConnectedToChannel() const { return pChannel != nullptr; }
    bool Empty() const { return !ConnectedToChannel() || pChannel->queue.empty(); }

};

template <typename T>
class Writer : public EndPoint<T> {

public:
    Writer() : EndPoint<T>() {};
    Writer(const Writer<T>& writer) : EndPoint<T>(writer) {}
    Writer(Writer<T>&& writer) : EndPoint<T>(writer) {}
    Writer(const Reader<T>& reader) : EndPoint<T>(reader) {}
    Writer(Channel<T>& channel) : EndPoint<T>(channel) {}

    ~Writer() {};

    Writer<T>& operator=(const Writer<T>& other) {
        this->pChannel = other.pChannel;
        return *this;
    }

    Writer<T>& operator=(Writer<T>&& other) {
        this->pChannel = move(other.pChannel);
        return *this;
    }

    bool Write(T&& message) {

        if (!this->ConnectedToChannel()) { return false; }

        lock_guard lock(this->pChannel->mutex);

        this->pChannel->queue.push(forward<T>(message));
        return true;
    }

};

template <typename T>
class Reader : public EndPoint<T> {
    
public:
    Reader() : EndPoint<T>() {};
    Reader(const Reader<T>& reader) : EndPoint<T>(reader) {}
    Reader(Reader<T>&& reader) : EndPoint<T>(reader) {}
    Reader(const Writer<T>& writer) : EndPoint<T>(writer) {}
    Reader(Channel<T>& channel) : EndPoint<T>(channel) {}

    ~Reader() {};

    Reader<T>& operator=(const Reader<T>& other) {
        this->pChannel = other.pChannel;
        return *this;
    }

    Reader<T>& operator=(Reader<T>&& other) {
        this->pChannel = move(other.pChannel);
        return *this;
    }

    T Read() {
        if (!this->ConnectedToChannel()) { return T(); }
        lock_guard lock(this->pChannel->mutex);

        if (this->pChannel->queue.empty()) { return T(); }

        T message = this->pChannel->queue.front();
        this->pChannel->queue.pop();

        return message;
    }
};