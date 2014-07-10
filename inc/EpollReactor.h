//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_EPOLLREACTOR_H
#define INCLUDED_EPOLLREACTOR_H

#include <sys/epoll.h>
#include <thread>
#include <array>
#include <iostream>
#include <stdexcept>
#include <string.h>
#include <unordered_map>

//------------------------------------------------------------------------------

namespace corvid { 

void throwErrno(const std::string& context = "") {
  throw std::runtime_error(context + ": " + std::string(strerror(errno)));
}

//------------------------------------------------------------------------------

struct FileDescriptor {
  FileDescriptor(int fd) 
    : fd_ (fd) {
  }

  virtual ~FileDescriptor() {
    //TODO close(*this);
  }
  
  operator int() const {
    return fd_;
  }

private:
  int fd_;  
};

struct EpollEvent {
  static const uint32_t read          = EPOLLIN; 
  static const uint32_t write         = EPOLLOUT; 
  static const uint32_t readHangup    = EPOLLRDHUP; 
  static const uint32_t priority      = EPOLLPRI; 
  static const uint32_t error         = EPOLLERR;
  static const uint32_t hangup        = EPOLLHUP; 
  static const uint32_t edgeTriggered = EPOLLET; 
  static const uint32_t oneShot       = EPOLLONESHOT; 
};

struct EpollReactor;
struct EpollHandler {
  virtual ~EpollHandler() {}
  virtual void onEvent(uint32_t events)             = 0;
};

struct EpollReactor : public FileDescriptor 
                    , public EpollHandler   {

  EpollReactor(size_t maxEvents = 20) 
  : FileDescriptor(epoll_create1(0)) 
  , maxEvents_   (maxEvents)
  , running_     (true)
  , epollThread_ (&EpollReactor::loop, this) {
    if (*this < 0) {
      throwErrno(__FUNCTION__);
    }
  }

  virtual ~EpollReactor() {
    running_ = false; 
    epollThread_.join();
  }

  template <typename Handler, typename...Args>
  void registerFd(FileDescriptor* pFd, uint32_t events, Args&&...args) {
    auto handler = handlers_.find(pFd);
    if (handler != handlers_.end()) {
      throw std::runtime_error("EpollReactor: cannot add already-registered fd");
    }

    EpollHandler* pHandler = new Handler(args...);

    epoll_event event;
    event.data.ptr = reinterpret_cast<void*>(pHandler);
    event.events   = events;

    epoll_ctl(*this, EPOLL_CTL_ADD, *pFd, &event);
    handlers_[pFd] = pHandler;
  }

  void modifyFd(FileDescriptor* pFd, uint32_t events) {
    auto handler = handlers_.find(pFd);
    if (handler == handlers_.end()) {
      throw std::runtime_error("EpollReactor: cannot modify non-existant fd");
    }

    epoll_event event;
    event.events = events;
    epoll_ctl(*this, EPOLL_CTL_MOD, *pFd, &event);
  }

  void removeFd(FileDescriptor* pFd) {
    auto handler = handlers_.find(pFd);
    if (handler == handlers_.end()) {
      throw std::runtime_error("EpollReactor: cannot remove non-existant fd");
    }

    epoll_event event;

    // TODO this is a race w/ the epoll thread...
    delete handler->second;
    handlers_.erase(handler);
    epoll_ctl(*this, EPOLL_CTL_DEL, *pFd, &event);
  }

  void terminate(EpollHandler* pHandler) {
    auto fd = getFdForHandler(pHandler);
    removeFd(fd);  
  }

  void loop () {
    while (running_) {
      try {
        static const int timeoutMs = 1000;
        int numEvents = epoll_wait(*this, events_.data(), maxEvents_, timeoutMs);
        while (numEvents-- > 0) {
          EpollHandler* pHandler = reinterpret_cast<EpollHandler*>(events_[numEvents].data.ptr);
          pHandler->onEvent(events_[numEvents].events);
        }
      }
      catch (std::exception& e) {
        std::cout << e.what() << std::endl;
      }
    }
  } 

  EpollHandler* getHandlerForFd(FileDescriptor* pFd) {
    auto handler = handlers_.find(pFd);
    if (handler == handlers_.end()) {
      throw std::runtime_error("EpollReactor: Cannot get handler for non-existant fd");
    }

    return handler->second;
  }

  // Much slower than fd -> handler lookup
  FileDescriptor* getFdForHandler(EpollHandler* pHandler) {
    for (auto handler: handlers_) {
      if (handler.second == pHandler) { 
        return handler.first; 
      }
    }
          
    throw std::runtime_error("EpollReactor: Cannot get fd for non-existant handler");
  }

protected:
  virtual void onEvent(uint32_t events) {
  }

private:
  typedef std::unordered_map<FileDescriptor*, EpollHandler*> HandlerMap;

  const size_t                        maxEvents_;
  std::array<epoll_event, 20>         events_;
  HandlerMap                          handlers_;
  bool                                running_;
  std::thread                         epollThread_;
};

//------------------------------------------------------------------------------

} // namespace corvid

//------------------------------------------------------------------------------


#endif
