from fastapi import FastAPI
from swagger_params import dealer_description, dealer_server_title, mo_server_title,\
mo_description, sd_server_title, sd_description

model_owner = FastAPI(title=mo_server_title,
                    description=mo_description)
super_dealer = FastAPI(title=sd_server_title,
                    description=sd_description)
dealer = FastAPI(title=dealer_server_title,
                description=dealer_description
                )

@model_owner.get("/model_inference")
def read_item(model_id: int):
    return {"batch_id": "1"}

@super_dealer.get("/batch_id")
def read_item(model_id: int):
    return {"batch_id": "1"}

@dealer.get("/seed")
def read_item(batch_id: str):
    return {"shares_seed": "123", "mac_key_mask": "123"}