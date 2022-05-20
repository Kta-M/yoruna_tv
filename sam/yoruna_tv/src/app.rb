require 'net/http'
require 'json'
require 'aws-sdk-ssm'

REMO_ENDPOINT = "https://api.nature.global"

def lambda_handler(event:, context:)
  token = ssm_param("YorunaTvRemoApiToken")

  url = "#{REMO_ENDPOINT}/1/appliances/#{ENV["APPLIANCE_ID"]}/tv"
  resp = Net::HTTP.post(
    URI.parse(url),
    "button=#{ENV["BUTTON_NAME"]}",
    { Authorization: "Bearer #{token}" }
  )

  puts resp.code
end

def ssm_param(name)
  client = Aws::SSM::Client.new(
    region: 'ap-northeast-1',
  )
  resp = client.get_parameter({
    name: name,
    with_decryption: true,
  })
  token = resp.dig(:parameter, :value)
end
