# Http-C
This is a simple implementation of HTTP/1.1 in C

## Feature Implementation
I asked an LLM "what a basic HTTP/1.1 server needs?" and this is the answer. I'm going to implement these for now. Everything that gets 'DONE✅' alongside the title, is implemented.
1. Basic HTTP/1.1 Request Handling
    Request Parsing: Ability to parse HTTP/1.1 requests, including method, URI, [headers](./HeaderParsingGuide.md), and body.
    Method Support: Handle common HTTP methods like GET, POST, PUT, DELETE, and HEAD.
2. Response Generation
    Status Codes: Support for generating responses with common HTTP status codes (e.g., 200 OK, 404 Not Found, 500 Internal Server Error).
    Headers: Ability to set response headers (e.g., Content-Type, Content-Length, Connection).
3. Connection Management
    Persistent Connections: Support for HTTP/1.1's default behavior of persistent connections using the Connection: keep-alive header.
    Connection Close: Handling of Connection: close headers to properly close connections when requested by the client.
4. Data Handling
    Chunked Transfer Encoding: Support for chunked transfer encoding for responses when the content length is unknown at the start of the response.
    Content Length: Proper handling of the Content-Length header for both requests and responses.
5. URI and Routing
    Path Parsing: Basic URL path parsing to route requests to the appropriate handlers.
    Routing: A mechanism to register and handle different routes and methods.
6. Error Handling
    Error Responses: Proper generation of error responses for common client and server errors.
    Logging: Basic logging for request handling and errors.
7. Multithreading or Multiprocessing
    Concurrency: Support for handling multiple simultaneous connections, either through threading, forking, or asynchronous I/O.
8. Security
    Basic Security Measures: Handling common security concerns like input validation and prevention of common attacks (e.g., header injection, path traversal).
9. Configuration
    Basic Configuration: Support for configuring server parameters such as port number, maximum connections, timeouts, etc.