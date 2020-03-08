# Restful_HttpServer
#基于meltwater/served库的一个简单web server

##served库
    Served is a C++ library for building high performance RESTful web servers. \<br>
    Served builds upon Boost.ASIO to provide a simple API for developers to create HTTP services in C++.

##Restful接口的设计与实现
    在基于本地资源的情况下，将本地向对外开放的资源统一存放与本地某一目录下，然后对外提供GET获取方法、基于GET方法的query查询接口，\<br>
还有任务状态更改的put方法。
    
