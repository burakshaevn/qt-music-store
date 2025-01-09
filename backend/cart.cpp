#include "cart.h"

Cart::Cart(std::shared_ptr<ProductCard> product_card, QObject *parent)
    : product_card_(std::move(product_card))
    , QObject{parent}
{}

void Cart::AddToCart(const QString& instrument_name){
    cart_[instrument_name] = product_card_->FindInstrumentCard(instrument_name);
}
void Cart::DeleteFromCart(const QString instrument_name){
    cart_.erase(cart_.find(instrument_name));
}
bool Cart::InstrumentInCart(const QString& instrument_name){
    return cart_.find(instrument_name) == cart_.end();
}
