# Assignment 2 Danger Log

## Other HTTP methods

In this homework, because we only implement Connect, post and Get method, so if another method is found we will just send the "HTTP/1.1 t found no method exist" to avoid further unkonwn errors happen.

## Implicit ports 

In some cases, the port will not be found in host part. Before the porxy returned the result port, we have to find out ports for these cases. So we implicitly add port "80" to http and '443' to https as the default ports to avoid danger happens.

## Receive response length limit

When recving response, sometimes it might exceeds the maximum limit of string. To avoid this happen, we changed the use of string to the  $vector<char>$, which has no length limit. However, we then face the question of how to use the  $vector<char>$ to the $recv()$ function, which take the $char *$ as a parameter. We then find out the $.data()$ function which can return the $char$ array, so we can return the address of specific position to the recv function.

## Receive response times limit

In some cases the to receive the response, it might need several times of calling $recv()$ function, while it wont be needed to receive the request, so we add a boolean $loop$ in the interface of the function $recv$ while it is recvRequest we then set the loop as false, then it only recv once, while it is recv response, it will continue to call $recv()$ until the size of results is 0.

## Multithreads danger to the Id of request

As the Id of every request is sent to every request constructor and after this process the Id counter in proxy should incre, when apply multi-threads to serve every received request，it might happen that several request has same Id. So to avoid it we add a mutelock while increasing the Id counter. And to avoid decrease on the efficiency, we just simply record and increase the Id first, then assign the recorded Id to construtor which requires no lock on it.


## empty contents while recving

While we use the function `recv()` to recv request or response, somtimes the content will be empty the the function will return a 0 `numbytes` .
Then while we parse the request and result, it will then crash. To avoid this happen, if we receive the empty response, the proxy will wait  until recv a next message with non-zero content. While for the request part, it will just send back a `400 bad request` error. As a result, the proxy will not crash due to empty requests/responses.

## `getaddrinfo()` ERROR or DNS failure

Some servers sometimes may not provide an available host to serve the requests, or the DNS resolver fails to resolve a domain name. In these cases, we traverse the linked list returned from `getaddrinfo`. If none of the hosts can be used for HTTP connection, we will rise a exception and the `serveconnection` function will print the failure and presents the reason.

## Bad requests and responses

Sometimes, when we receive the corrupted request and response, if we can parse it and expected action happened, then we throw it in the form of `myExcepetion`, then the related actions can be taken. However, if we cannot parse it properly, then other exception types will be thwon we then use `cath(...)` to catch it and send back the corresponding `400 bad request` or `500 bad response` to the client.

## different HTTP status codes

Aside from common http response, such as 200 or 304 or 502, we also handled some other kinds of responses. When the url cannot be found, our proxy will send 404 NOT FOUND. When we receive methods not implemented in this homework, 501 Not Implemented is sent back to the client.

## cache updates


## memory management
We use some C++11 STL features and data structures and are very cautious about creating objects with `new`, we only use `new` to create the request class and delete it once we finish the processing.
