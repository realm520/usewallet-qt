#include "ErrorTranslator.h"
#include <QObject>
 QString ErrorTranslator::Translate(QString src)
{
	auto it=getErrorTranslator().translate_map.find(src);
	if (it == getErrorTranslator().translate_map.end())
		return src;
	return it.value();
}

 ErrorTranslator & ErrorTranslator::getErrorTranslator()
{

	static ErrorTranslator* instance = NULL;
	if (instance == NULL)
		instance = new ErrorTranslator();
	return *instance;
}

inline ErrorTranslator::ErrorTranslator()
{
	reset_translate_map();
}

void ErrorTranslator::reset_translate_map()
{
	translate_map.clear();
	translate_map["wallet error"] = QObject::tr("wallet error");
	translate_map["invalid password"] = QObject::tr("invalid password");
	translate_map["wallet locked"] = QObject::tr("wallet locked");
	translate_map["wallet already exists"] = QObject::tr("wallet already exists");
	translate_map["wallet does not exist"] = QObject::tr("wallet does not exist");
	translate_map["unknown local account"] = QObject::tr("unknown local account");
	translate_map["unknown account"] = QObject::tr("unknown account");
	translate_map["wallet closed"] = QObject::tr("wallet closed");
	translate_map["negative bid"] = QObject::tr("negative bid");
	translate_map["invalid price"] = QObject::tr("invalid price");
	translate_map["insufficient funds"] = QObject::tr("insufficient funds");
	translate_map["unknown market order"] = QObject::tr("unknown market order");
	translate_map["fee greater than amount"] = QObject::tr("fee greater than amount");
	translate_map["unknown address"] = QObject::tr("unknown address");
	translate_map["brain key is too short"] = QObject::tr("brain key is too short");
	translate_map["password too short"] = QObject::tr("password too short");
	translate_map["invalid timestamp"] = QObject::tr("invalid timestamp");
	translate_map["invalid account name"] = QObject::tr("invalid account name");
	translate_map["file already exists"] = QObject::tr("file already exists");
	translate_map["file not found"] = QObject::tr("file not found");
	translate_map["invalid timeout"] = QObject::tr("invalid timeout");
	translate_map["invalid operation"] = QObject::tr("invalid operation");
	translate_map["transaction not found"] = QObject::tr("transaction not found");
	translate_map["invalid transaction id"] = QObject::tr("invalid transaction id");
	translate_map["invalid asset symbol"] = QObject::tr("invalid asset symbol");
	translate_map["duplicate account name"] = QObject::tr("duplicate account name");
	translate_map["not contact account"] = QObject::tr("not contact account");
	translate_map["memo too long"] = QObject::tr("memo too long");
	translate_map["invalid pay rate"] = QObject::tr("invalid pay rate");
	translate_map["invalid wallet name"] = QObject::tr("invalid wallet name");
	translate_map["unsupported version"] = QObject::tr("unsupported version");
	translate_map["invalid fee"] = QObject::tr("invalid fee");
	translate_map["key belongs to other account"] = QObject::tr("key belongs to other account");
	translate_map["double cover"] = QObject::tr("double cover");
	translate_map["stupid order"] = QObject::tr("stupid order");
	translate_map["invalid expiration time"] = QObject::tr("invalid expiration time");
	translate_map["bad collateral amount"] = QObject::tr("bad collateral amount");
	translate_map["unknown slate"] = QObject::tr("unknown slate");
	translate_map["duplicate key"] = QObject::tr("duplicate key");
	translate_map["unauthorized child account"] = QObject::tr("unauthorized child account");
	translate_map["invalid slate"] = QObject::tr("invalid slate");
	translate_map["private key not found"] = QObject::tr("private key not found");
	translate_map["invalid cancel"] = QObject::tr("invalid cancel");
	translate_map["account not registered"] = QObject::tr("account not registered");
	translate_map["invalid contact"] = QObject::tr("invalid contact");
	translate_map["label already in use"] = QObject::tr("label already in use");
	translate_map["account retracted"] = QObject::tr("account retracted");
	translate_map["asset issuer not found"] = QObject::tr("asset issuer not found");
	translate_map["the authority address to remove not found"] = QObject::tr("the authority address to remove not found");
	translate_map["the authority address not supplied"] = QObject::tr("the authority address not supplied");
	translate_map["Blockchain Exception"] = QObject::tr("Blockchain Exception");
	translate_map["invalid pts address"] = QObject::tr("invalid pts address");
	translate_map["addition overflow"] = QObject::tr("addition overflow");
	translate_map["subtraction overflow"] = QObject::tr("subtraction overflow");
	translate_map["asset/price mismatch"] = QObject::tr("asset/price mismatch");
	translate_map["unsupported chain operation"] = QObject::tr("unsupported chain operation");
	translate_map["unknown transaction"] = QObject::tr("unknown transaction");
	translate_map["duplicate transaction"] = QObject::tr("duplicate transaction");
	translate_map["zero amount"] = QObject::tr("zero amount");
	translate_map["zero price"] = QObject::tr("zero price");
	translate_map["asset divide by self"] = QObject::tr("asset divide by self");
	translate_map["asset divide by zero"] = QObject::tr("asset divide by zero");
	translate_map["new database version"] = QObject::tr("new database version");
	translate_map["unlinkable block"] = QObject::tr("unlinkable block");
	translate_map["price out of range"] = QObject::tr("price out of range");
	translate_map["block numbers not sequential"] = QObject::tr("block numbers not sequential");
	translate_map["invalid previous block"] = QObject::tr("invalid previous block");
	translate_map["invalid block time"] = QObject::tr("invalid block time");
	translate_map["time is in the past"] = QObject::tr("time is in the past");
	translate_map["time is in the future"] = QObject::tr("time is in the future");
	translate_map["invalid block digest"] = QObject::tr("invalid block digest");
	translate_map["invalid delegate signee"] = QObject::tr("invalid delegate signee");
	translate_map["failed checkpoint verification"] = QObject::tr("failed checkpoint verification");
	translate_map["wrong chain id"] = QObject::tr("wrong chain id");
	translate_map["unknown block"] = QObject::tr("unknown block");
	translate_map["block is older than our undo history allows"] = QObject::tr("block is older than our undo history allows us to process");
	translate_map["unexpected supply change"] = QObject::tr("unexpected supply change");
	translate_map["invalid price"] = QObject::tr("invalid price");
	translate_map["invalid apr"] = QObject::tr("invalid apr");
	translate_map["invalid amount"] = QObject::tr("invalid amount");
	translate_map["Evaluation Error"] = QObject::tr("Evaluation Error");
	translate_map["negative deposit"] = QObject::tr("negative deposit");
	translate_map["not a delegate"] = QObject::tr("not a delegate");
	translate_map["unknown balance record"] = QObject::tr("unknown balance record");
	translate_map["insufficient funds"] = QObject::tr("insufficient funds");
	translate_map["missing signature"] = QObject::tr("missing signature");
	translate_map["invalid claim password"] = QObject::tr("invalid claim password");
	translate_map["invalid withdraw condition"] = QObject::tr("invalid withdraw condition");
	translate_map["negative withdraw"] = QObject::tr("negative withdraw");
	translate_map["not an active delegate"] = QObject::tr("not an active delegate");
	translate_map["expired transaction"] = QObject::tr("expired transaction");
	translate_map["invalid transaction expiration"] = QObject::tr("invalid transaction expiration");
	translate_map["transaction exceeded the maximum transaction"] = QObject::tr("transaction exceeded the maximum transaction size");
	translate_map["balance must bigger or equal to 0"] = QObject::tr("balance must bigger or equal to 0");
	translate_map["asset in deposit_to_asset_operation must be"] = QObject::tr("asset in deposit_to_asset_operation must be used up");
	translate_map["message which carried by trx is too long"] = QObject::tr("message which carried by trx is too long");
	translate_map["invalid account name"] = QObject::tr("invalid account name");
	translate_map["unknown account id"] = QObject::tr("unknown account id");
	translate_map["unknown account name"] = QObject::tr("unknown account name");
	translate_map["missing parent account signature"] = QObject::tr("missing parent account signature");
	translate_map["parent account retracted"] = QObject::tr("parent account retracted");
	translate_map["account expired"] = QObject::tr("account expired");
	translate_map["account already registered"] = QObject::tr("account already registered");
	translate_map["account key already in use"] = QObject::tr("account key already in use");
	translate_map["account retracted"] = QObject::tr("account retracted");
	translate_map["unknown parent account name"] = QObject::tr("unknown parent account name");
	translate_map["unknown delegate slate"] = QObject::tr("unknown delegate slate");
	translate_map["too many delegates in slate"] = QObject::tr("too many delegates in slate");
	translate_map["pay balance remaining"] = QObject::tr("pay balance remaining");
	translate_map["unknown_account"] = QObject::tr("unknown_account");
	translate_map["invalid_pay_rate"] = QObject::tr("invalid_pay_rate");
	translate_map["not delegates signature"] = QObject::tr("not delegates signature");
	translate_map["invalid precision"] = QObject::tr("invalid precision");
	translate_map["invalid asset symbol"] = QObject::tr("invalid asset symbol");
	translate_map["unknown asset id"] = QObject::tr("unknown asset id");
	translate_map["asset symbol in use"] = QObject::tr("asset symbol in use");
	translate_map["invalid asset amount"] = QObject::tr("invalid asset amount");
	translate_map["negative issue"] = QObject::tr("negative issue");
	translate_map["over issue"] = QObject::tr("over issue");
	translate_map["unknown asset symbol"] = QObject::tr("unknown asset symbol");
	translate_map["asset id in use"] = QObject::tr("asset id in use");
	translate_map["not user issued"] = QObject::tr("not user issued");
	translate_map["invalid asset name"] = QObject::tr("invalid asset name");
	translate_map["amount too large"] = QObject::tr("amount too large");
	translate_map["outstanding shares exist"] = QObject::tr("outstanding shares exist");
	translate_map["invalid fee rate"] = QObject::tr("invalid fee rate");
	translate_map["invalid authority"] = QObject::tr("invalid authority");
	translate_map["permission not available"] = QObject::tr("permission not available");
	translate_map["invalid whitelist"] = QObject::tr("invalid whitelist");
	translate_map["not on whitelist"] = QObject::tr("not on whitelist");
	translate_map["market halted"] = QObject::tr("market halted");
	translate_map["account is not allow to create asset"] = QObject::tr("account is not allow to create asset");
	translate_map["delegate_vote_limit"] = QObject::tr("delegate_vote_limit");
	translate_map["insufficient fee"] = QObject::tr("insufficient fee");
	translate_map["negative fee"] = QObject::tr("negative fee");
	translate_map["missing deposit"] = QObject::tr("missing deposit");
	translate_map["insufficient relay fee"] = QObject::tr("insufficient relay fee");
	translate_map["fee greater than max"] = QObject::tr("fee greater than max");
	translate_map["invalid market"] = QObject::tr("invalid market");
	translate_map["unknown market order"] = QObject::tr("unknown market order");
	translate_map["shorting base shares"] = QObject::tr("shorting base shares");
	translate_map["insufficient collateral"] = QObject::tr("insufficient collateral");
	translate_map["insufficient depth"] = QObject::tr("insufficient depth");
	translate_map["insufficient feeds"] = QObject::tr("insufficient feeds");
	translate_map["invalid feed price"] = QObject::tr("invalid feed price");
	translate_map["price multiplication overflow"] = QObject::tr("price multiplication overflow");
	translate_map["price multiplication underflow"] = QObject::tr("price multiplication underflow");
	translate_map["price multiplication undefined product 0*inf"] = QObject::tr("price multiplication undefined product 0*inf");
}
