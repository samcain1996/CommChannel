#include <queue>
#include <mutex>
#include <set>

using namespace std;

template <typename T>
struct EndPoint;

template <typename T>
struct Channel {

public:
    mutex mutex;
    queue<T> queue;

    Channel() { }

    bool ConnectEndPoint(EndPoint<T>& endpoint) {

       // if (endpoint.ConnectedToChannel()) { return false; }

        endpoint.pChannel = this;
        return true;
    }
};

template <typename T>
class EndPoint {

    friend class Channel<T>;

protected:

    Channel<T>* pChannel = nullptr;

    EndPoint() {};
    EndPoint(const EndPoint<T>& endpoint) {
        pChannel = endpoint.pChannel;
    }
    EndPoint(EndPoint<T>&& endpoint) {
        pChannel = move(endpoint.pChannel);
    }

    ~EndPoint() {};

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

};

template <typename T>
class Writer : public EndPoint<T> {
    public:
    Writer() : EndPoint<T>() {};
    Writer(const Writer<T>& writer) : EndPoint<T>(writer) {}
    Writer(Writer<T>&& writer) : EndPoint<T>(writer) {}

    ~Writer() {};

    Writer<T>& operator=(const Writer<T>& other) {
        this->pChannel = other.pChannel;
        return *this;
    }

    Writer<T>& operator=(Writer<T>&& other) {
        this->pChannel = move(other.pChannel);
        return *this;
    }

    void Write(T&& message) {

        if (!this->ConnectedToChannel()) { return; }

        lock_guard lock(this->pChannel->mutex);

        this->pChannel->queue.push(forward<T>(message));
    }

};

template <typename T>
class Reader : public EndPoint<T> {
    public:
    Reader() : EndPoint<T>() {};
    Reader(const Reader<T>& reader) : EndPoint<T>(reader) {}
    Reader(Reader<T>&& reader) : EndPoint<T>(reader) {}
    Reader(const Writer<T>& writer) : EndPoint<T>(writer) {}

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