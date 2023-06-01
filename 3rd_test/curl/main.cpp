
#include <iostream>
#include <curl/curl.h>

// 回调函数，用于接收和处理 HTTP 响应数据
size_t WriteCallback(char* contents, size_t size, size_t nmemb, std::string* output) 
{
	size_t totalSize = size * nmemb;
	output->append(contents, totalSize);
	return totalSize;
}

int main() {
	// 初始化 libcurl
	curl_global_init(CURL_GLOBAL_DEFAULT);

	// 创建一个 CURL 对象
	CURL* curl = curl_easy_init();
	if (curl) 
	{
		std::string response;

		// 设置请求的 URL
		curl_easy_setopt(curl, CURLOPT_URL, "https://www.baidu.com");
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
		// 设置接收响应数据的回调函数
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);


		// 发送 HTTP GET 请求
		CURLcode res = curl_easy_perform(curl);
		if (res != CURLE_OK) 
		{
			std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
		}
		else 
		{
			// 打印响应数据
			std::cout << "Response: " << response << std::endl;
		}

		// 清理 CURL 对象
		curl_easy_cleanup(curl);
	}

	// 清理 libcurl
	curl_global_cleanup();

	system("pause");

	return 0;
}
