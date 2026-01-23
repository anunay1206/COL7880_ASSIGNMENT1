#include "functions.h"
#include <omp.h>

struct OrderBookEntry {
    uint32_t stockID;
    bool orderType; 
    uint8_t orderQty;
    uint8_t orderValue;
};

OrderBookEntry decodePacket(uint64_t packet) {
    OrderBookEntry entry;
    entry.stockID = (packet >> 0) & 0xFFFFFFFF;
    entry.orderType = ((packet >> 32) & 0x1) == 1;
    entry.orderQty = (packet >> 33) & 0xFF;
    entry.orderValue = (packet >> 41) & 0xFF;
    
    return entry;
}

uint64_t unstuffBits(uint64_t packet) {
    uint64_t result = 0;
    int outputBitPos = 0;
    int consecutiveOnes = 0;
    const int TARGET_BITS = 49;
    
    int i = 0;
    while (i < 64 && outputBitPos < TARGET_BITS) {
        bool bit = (packet >> i) & 0x1;
        
        if (bit) {
            consecutiveOnes++;
            result |= (1ULL << outputBitPos);
            outputBitPos++;
            
            if (consecutiveOnes == 5) {
                consecutiveOnes = 0;
                i++; 
                continue;
            }
        } else {
            consecutiveOnes = 0;
            outputBitPos++;
        }
        i++;
    }
    
    return result;
}

struct StockInfo {
    uint32_t stockID;
    uint8_t lastSellValue;
    uint8_t lastBuyValue;
    uint8_t minSellValue;
    uint8_t maxBuyValue;
    int64_t sumOrderValue;  
    int orderCount;         
    bool hasSellForMin;     
    bool hasBuyForMax;
    int64_t lastSellTime;
    int64_t lastBuyTime;
    
    StockInfo() : lastSellValue(0), lastBuyValue(0), 
                  minSellValue(255), maxBuyValue(0),
                  sumOrderValue(0), orderCount(0),
                  lastSellTime(-1), lastBuyTime(-1),
                  hasSellForMin(false), hasBuyForMax(false) {}
    
    int getSpread() const {
        return abs((int)lastSellValue - (int)lastBuyValue);
    }
};

void updateDisplay(const std::vector<uint64_t> &orderBook, int32_t freq) {
    size_t n = orderBook.size();
    int chunks = (n + freq - 1) / freq;
    std:: vector<std:: map<uint32_t, StockInfo>> chunkState(chunks);


    #pragma omp parallel for

    for(int b = 0;b < chunks;b++)
    {
        size_t start = (size_t)b * freq;
        size_t end = std::min(start + freq, n);

        std::map<uint32_t, StockInfo>&localMap = chunkState[b];
        
        for(size_t i = start; i<end; i++)
        {
            uint64_t unstuffed = unstuffBits(orderBook[i]);
            OrderBookEntry entry = decodePacket(unstuffed);

            int64_t timestamp = i;

            StockInfo &stock = localMap[entry.stockID];
            stock.stockID = entry.stockID;

            if(entry.orderType)
            {
                if(timestamp > stock.lastSellTime)
                {
                    stock.lastSellTime = timestamp;
                    stock.lastSellValue = entry.orderValue;
                }
                stock.hasSellForMin = true;
                stock.minSellValue = std :: min(stock.minSellValue, entry.orderValue);
            }
            else{
                if(timestamp > stock.lastBuyTime)
                {
                    stock.lastBuyTime = timestamp;
                    stock.lastBuyValue = entry.orderValue;
                }
                stock.hasBuyForMax = true;
                stock.maxBuyValue = std::max(stock.maxBuyValue, entry.orderValue);
            }
        }
    }

    std :: map<uint32_t,StockInfo> finalMap;

    int snapCount = 0;
    for(int b=0;b<chunks;b++)
    {
        for(auto &j : chunkState[b])
        {
            StockInfo &dest = finalMap[j.first];
            dest.stockID = j.first;
            StockInfo &src = j.second;

            if(src.lastBuyTime>dest.lastBuyTime)
            {
                dest.lastBuyTime = src.lastBuyTime;
                dest.lastBuyValue = src.lastBuyValue;
            }

            if(src.lastSellTime >dest.lastSellTime)
            {
                dest.lastSellTime = src.lastSellTime;
                dest.lastSellValue = src.lastSellValue;
            }

            if(src.hasSellForMin)
            {
                dest.hasSellForMin = true;
                dest.minSellValue = std::min(dest.minSellValue, src.minSellValue);
            }
            
            if(src.hasBuyForMax)
            {
                dest.hasBuyForMax = true;
                dest.maxBuyValue = std::max(dest.maxBuyValue, src. maxBuyValue);
            }
        }

        std::vector<StockInfo> stocks;
        for (auto& pair : finalMap) {
            stocks.push_back(pair.second);
        }
        
        std::sort(stocks.begin(), stocks.end(), [](const StockInfo& a, const StockInfo& b) {
            int spreadA = a.getSpread();
            int spreadB = b.getSpread();
            if (spreadA != spreadB) {
                return spreadA > spreadB;
            }
            return a.stockID > b.stockID;
        });
            
            std::string filename =
    OUTPUT_DIR + "/snap_" + std::to_string(snapCount++) + ".txt";
            std::ofstream outFile(filename);
            
            for (const auto& stock : stocks) {
                int spread = stock.getSpread();
                outFile << stock.stockID << " " 
                        << (int)stock.lastSellValue << " " 
                        << (int)stock.lastBuyValue << " " 
                        << spread << "\n";
            }
            
            outFile.close();
    }
}

int64_t totalAmountTraded(const std::vector<uint64_t> &orderBook) {
    int64_t total = 0;
    
    #pragma omp parallel for reduction(+:total)
    for (uint64_t packet : orderBook) {
        uint64_t unstuffed = unstuffBits(packet);
        OrderBookEntry entry = decodePacket(unstuffed);
        
        total += (int64_t)entry.orderQty * (int64_t)entry.orderValue;
    }
    
    return total;
}

void printOrderStats(const std::vector<uint64_t> &orderBook) {

    int numThreads = omp_get_max_threads();
    std::vector<std::map<uint32_t, StockInfo>> localMaps(numThreads);

    #pragma omp parallel for 

    for(size_t i = 0; i<orderBook.size();i++)
    {
        int tid = omp_get_thread_num();

        uint64_t unstuffed = unstuffBits(orderBook[i]);
        OrderBookEntry entry = decodePacket(unstuffed);

        StockInfo &stock = localMaps[tid][entry.stockID];
        stock.stockID = entry.stockID;

        if (entry.orderType) {
            stock.hasSellForMin = true;
            if (entry.orderValue < stock.minSellValue) {
                stock.minSellValue = entry.orderValue;
            }
        } else {
            stock.hasBuyForMax = true;
            if (entry.orderValue > stock.maxBuyValue) {
                stock.maxBuyValue = entry.orderValue;
            }
        }
        
        stock.sumOrderValue += (int64_t)entry.orderValue;
        stock.orderCount++;
    }
    
    std::map<uint32_t, StockInfo> finalMap;

    for(int t=0;t<numThreads;t++)
    {
        for(auto &pair : localMaps[t])
        {
            uint32_t stockID = pair.first;
            StockInfo &localStock = pair.second;
            StockInfo &finalStock = finalMap[stockID];

            finalStock.stockID = stockID;

            if(localStock.hasSellForMin)
            {
                finalStock.hasSellForMin = true;
                finalStock.minSellValue = std::min(finalStock.minSellValue, localStock.minSellValue);
            }
            if(localStock.hasBuyForMax)
            {
                finalStock.hasBuyForMax = true;
                finalStock.maxBuyValue = std::max(finalStock.maxBuyValue, localStock.maxBuyValue);
            }

            finalStock.sumOrderValue += localStock.sumOrderValue;
            finalStock.orderCount += localStock.orderCount;
        }
    }

    
    std::ofstream outFile(OUTPUT_DIR + "/stats.txt");
    outFile << std::fixed << std::setprecision(4);
    
    for (auto& pair : finalMap) {
        const StockInfo& stock = pair.second;
            
        double avgOrderValue = 0.0;
        if (stock.orderCount > 0) {
            avgOrderValue = static_cast<double>(stock.sumOrderValue) /
                            static_cast<double>(stock.orderCount);
        }
            
        uint8_t minSell = stock.hasSellForMin ? stock.minSellValue : 0;
        uint8_t maxBuy  = stock.hasBuyForMax  ? stock.maxBuyValue  : 0;
    
        outFile << stock.stockID << " "
                << static_cast<int>(minSell) << " "
                << static_cast<int>(maxBuy) << " "
                << std::fixed << std::setprecision(3)
                << avgOrderValue << "\n";
    }
    
    outFile.close();
}
