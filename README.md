# HTTP Proxy
Created a proxy that uses TCP sockets and intercepts all HTTP requests and responses between a browser and web server. The proxy redirects each GET request for a JPG image to one of two file paths containing a PNG image. Additionally, the proxy replaces a keyword within the HTML message of a web server response with another word. The proxy was developed with C/C++ on Linux. The proxy was tested on the following site which has 6 test cases using a Firefox browser: https://pages.cpsc.ucalgary.ca/~carey/CPSC441/assignment1.html

## User Manual:
1. Compile clown.cpp with the following command on a CPSC Linux server:
   
   g++ -o clown clown.cpp

2. Execute the program with your choice of port number as the command line argument. For this
example, we will choose port number 11140:
    
    ./clown 11140

3. Open the Firefox browser on https://pages.cpsc.ucalgary.ca/~carey/CPSC441/assignment1.html

4. Choose “settings” from the three horizontal lines menu at the top right and go to “Network
Settings” (can also type in “proxy” in the search bar). If needed, select “clear data” under
“cookies and site data” under “settings”.

5. Select “Manual proxy configuration”

6. Select “HTTP Proxy” and input the IP address and selected port. For example, 136.159.5.27 for
csx3.cpsc.ucalgary.ca and port number 11140

7. Select “Ok”

