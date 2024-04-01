#include <iostream>
#include <vector>
#include <condition_variable>
#include <fstream>
#include <SFML/Graphics.hpp>
#include "ImageGrid.h"
#include "Downloader.h"

#include <future>

std::chrono::steady_clock::time_point startTime;

void downloadAndLoadImage(std::string url, sf::Texture& texture, CDownloader& downloader) {
    std::string filePath = "Images/" + url.substr(url.find_last_of('/') + 1);
    std::ifstream file(filePath);
    if (file.good()) {
        if (texture.loadFromFile(filePath)) {
            std::cout << "Loaded from file: " << filePath << std::endl;
            return;
        }
    }
    // Failed to load the file will download instead
    if (downloader.DownloadToFile(url.c_str(), filePath.c_str())) {
        if (texture.loadFromFile(filePath)) {
            std::cout << "Downloaded and loaded: " << url << std::endl;
            return;
        }
    }

    // Failed to download or load the file
    std::cerr << "Failed to download image: " << url << std::endl;
}


void screenshot(const std::string& fileSaveLocation, sf::Window* window) {
    sf::Texture texture;
    texture.create(window->getSize().x, window->getSize().y);
    texture.update(*window);
    if (texture.copyToImage().saveToFile(fileSaveLocation)) {
        std::cout << "Screenshot saved to " << fileSaveLocation << std::endl;
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "GD2P03 Assignment 1");

    ImageGrid imagegrid(100);

    std::string data;
    CDownloader downloader;
    downloader.Init();

    if (!downloader.Download("https://raw.githubusercontent.com/MDS-HugoA/TechLev/main/ImgListSmall.txt", data)) {
        std::cerr << "Data failed to download";
        return -1;
    }

    std::vector<std::string> urls;
    int oldPos = 0;
    while (oldPos < data.length()) {
        int pos = data.find('\n', oldPos);
        urls.emplace_back(data.substr(oldPos, pos - oldPos));
        oldPos = pos + 1;
    }

    startTime = std::chrono::steady_clock::now();

    std::vector<sf::Texture> textures;
    textures.reserve(urls.size());

    std::vector<std::future<void>> futures;
    for (const auto& url : urls) {
        textures.emplace_back();
        futures.push_back(std::async(std::launch::async, downloadAndLoadImage, url, std::ref(textures.back()), std::ref(downloader)));
    }
    for (auto& future : futures) {
        future.wait(); // Wait for each future to finish execution
    }

    for (int i = 0; i < 10; i++)
    {
        imagegrid.addTile(textures[i]);
    }
    imagegrid.RepositionTiles(3);
   ////creation of images
   //int gridSize = 3; //3x3
   //int tileSize = 100;
   //std::vector<std::vector<sf::RectangleShape>> imageGrid(gridSize, std::vector<sf::RectangleShape>(gridSize));
   //int count = 0;
   //for (int x = 0; x < 3; x++) {
   //    for (int y = 0; y < 3; y++) {
   //        imageGrid[x][y].setTexture(&textures[count]);
   //        imageGrid[x][y].setSize(sf::Vector2f(tileSize, tileSize));
   //        imageGrid[x][y].setPosition(x * tileSize, y * tileSize);
   //        count++;
   //    }
   //}

    auto endTime = std::chrono::steady_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    std::cout << "Total time taken to load images: " << elapsedTime << " milliseconds" << std::endl;


    while (window.isOpen()) {
        sf::Event winEvent;
        while (window.pollEvent(winEvent)) {
            if (winEvent.type == sf::Event::Closed) {
                screenshot("Images/combinedImage.png", &window);
                window.close();
            }
        }

        window.clear();
        //for (const auto& row : imageGrid) {
        //    for (const auto& image : row) {
        //        window.draw(image);
        //    }
        //}
        imagegrid.Draw(window);

        window.display();
    }

    return 0;
}
