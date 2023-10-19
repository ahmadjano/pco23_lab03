#include "wholesale.h"
#include "factory.h"
#include "costs.h"
#include <iostream>
#include <pcosynchro/pcothread.h>

WindowInterface* Wholesale::interface = nullptr;

Wholesale::Wholesale(int uniqueId, int fund)
    : Seller(fund, uniqueId)
{
    interface->updateFund(uniqueId, fund);
    interface->consoleAppendText(uniqueId, "Wholesaler Created");

}

void Wholesale::setSellers(std::vector<Seller*> sellers) {
    this->sellers = sellers;

    for(Seller* seller: sellers){
        interface->setLink(uniqueId, seller->getUniqueId());
    }
}

void Wholesale::buyResources() {
    auto s = Seller::chooseRandomSeller(sellers);
    auto m = s->getItemsForSale();
    auto i = Seller::chooseRandomItem(m);

    if (i == ItemType::Nothing) {
        /* Nothing to buy... */
        return;
    }

    int qty = rand() % 5 + 1;
    int price = qty * getCostPerUnit(i);

    interface->consoleAppendText(uniqueId, QString("I would like to buy %1 of ").arg(qty) %
                                 getItemName(i) % QString(" which would cost me %1").arg(price));

    // Ensure that we have enough money to buy what we want.
    if (price > money) {
        interface->consoleAppendText(uniqueId, QString("Not enough money"));
        return;
    }

    // Request the trade to Extractor/Factory.
    int facture = s->trade(i, qty);

    // If the seller accepted the purchase.
    if (facture > 0) {
        money -= facture; // `facture` should be equal to `price` here.
        stocks.at(i) += qty;

        interface->consoleAppendText(uniqueId, QString("Bought %1 ").arg(qty) %
                                     getItemName(i) % QString(" for %1").arg(price));
    } else {
        interface->consoleAppendText(uniqueId, QString("Seller has shortage in stock."));
    }

}

void Wholesale::run() {

    if (sellers.empty()) {
        std::cerr << "You have to give factories and mines to a wholeseler before launching is routine" << std::endl;
        return;
    }

    interface->consoleAppendText(uniqueId, "[START] Wholesaler routine");
    while (true /* TODO terminaison*/) {
        buyResources();
        interface->updateFund(uniqueId, money);
        interface->updateStock(uniqueId, &stocks);
        //Temps de pause pour espacer les demandes de ressources
        PcoThread::usleep((rand() % 10 + 1) * 100000);
    }
    interface->consoleAppendText(uniqueId, "[STOP] Wholesaler routine");


}

std::map<ItemType, int> Wholesale::getItemsForSale() {
    return stocks;
}

int Wholesale::trade(ItemType it, int qty) {
    if (stocks[it] < qty) {
        return 0;
    }

    // Accept the purchase otherwise.
    int cost = getCostPerUnit(it) * qty;

    getItemsForSale().at(it) -= qty; // Update the stock.
    money += cost;

    return cost;
}

void Wholesale::setInterface(WindowInterface *windowInterface) {
    interface = windowInterface;
}
