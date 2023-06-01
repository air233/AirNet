
#include <iostream>
#include <curl/curl.h>

// �ص����������ڽ��պʹ��� HTTP ��Ӧ����
size_t WriteCallback(char* contents, size_t size, size_t nmemb, std::string* output) 
{
	size_t totalSize = size * nmemb;
	output->append(contents, totalSize);
	return totalSize;
}

int main() {
	// ��ʼ�� libcurl
	curl_global_init(CURL_GLOBAL_DEFAULT);

	// ����һ�� CURL ����
	CURL* curl = curl_easy_init();
	if (curl) 
	{
		std::string response;

		// ��������� URL
		curl_easy_setopt(curl, CURLOPT_URL, "https://www.baidu.com");
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
		// ���ý�����Ӧ���ݵĻص�����
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);


		// ���� HTTP GET ����
		CURLcode res = curl_easy_perform(curl);
		if (res != CURLE_OK) 
		{
			std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
		}
		else 
		{
			// ��ӡ��Ӧ����
			std::cout << "Response: " << response << std::endl;
		}

		// ���� CURL ����
		curl_easy_cleanup(curl);
	}

	// ���� libcurl
	curl_global_cleanup();

	system("pause");

	return 0;
}
