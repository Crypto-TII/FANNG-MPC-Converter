
# Swagger API parameters!
dealer_server_title = "Dealer's REST server!"
dealer_description =  """
            Dealer REST server to handle Client's /seed request. 🚀

            Retrieve seed and mack key mask for a given batch ID.

            Dealer looks for the seeds and masks in its databses and 
            if found returns the clients with this information or else 
            sends an empty response and the client waits and keeps pinging
            this server every 5 seconds for seeds and mac key mask.

            Parameters:
            - `batch_id`: `int` (0 to 2147483647), 

                batch_id for which seed and mack key mask are requested.

            Returns:
            - JSON response containing the shares seed and MAC key mask if batch_id is found.

                `Note`: If the dealer doesn't have the particular batch_id information then the client is
                sent the response: "seed preparation not done".

            """
mo_server_title = "Model Owner's REST server!"
mo_description =  """
            Model Owner's REST server to handle Client's /model_inference request. 🚀

            Retrieve batch_id for the requested model_id by raising a request
            to the super dealer.

            Model Owner after receiving the model_id from the client requests the 
            Super Dealer with the same model_id to get information on whether the 
            batch_id is available for that particular model_id and if found returns
            to the Clients. 

            Parameters:
            - `model_id`: `int` (0 to 2147483647), 

                model_id for which the batch_id is requested.

            Returns:
            - JSON response containing the batch_id if returned by Super Dealer.

                `Note`: If the super dealer doesn't return a batch_id to the model owner
                to return to the clients then the client raises an Error and stops its process".

            """
sd_server_title = "Super Dealer's REST server!"
sd_description =  """
            Super Dealer's REST server handles Model Owner's `/batch` request. 🚀

            Retrieve batch_id for desired model_id for which the material processing
            is finished by the Model Owner.

            Super Dealer contains in its databases the batch_ids for various model_ids. 
            If the Super Dealer has the batch_id for the requested model_id then it returns
            it to the model owner.

            Parameters:
            - `model_id`: `int` (0 to 2147483647), 

                model_id for which the batch_id is requested.

            Returns:
            - JSON response containing the batch_id if found in Super Dealer's databases.

                `Note`: If the super dealer doesn't have the particular batch_id information
                then the model_inference request is terminated.

            """