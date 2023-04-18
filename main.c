#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include <string.h>
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "ii.h"
#include "dhcpserver.h"
#include "dnsserver.h"
#include <lwip/udp.h>
#include "lwip/pbuf.h"
#include "lwip/sockets.h"

/* I2c pins on Pico */
#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9

// Define the UDP port to listen on
#define UDP_PORT 9301

// Define the maximum length of incoming OSC messages
#define MAX_OSC_LEN 1024

Module_info* retrieve_module(char* slice_module_name) {
  if(strcmp(slice_module_name, "er301") == 0) {
    printf("er301\n*******\n");
    return &er301_info;
  } else if(strcmp(slice_module_name, "txo") == 0) {
    printf("txo\n********\n");
    return &txo_info;
  }
  return NULL;
}

II_command* retrieve_cmd(Module_info* module_info, char* command) {
  int i;
  for(i = 0; i < module_info->number_of_cmd; i++) {
    if(strcmp(module_info->cmd_set[i].name, command) == 0) {
      return &module_info->cmd_set[i];
    }
  }
  return NULL;
}


// add something to check the max number of ports
void parseData(char packet[]) {

  // parse and check data received over OSC. Prepare data that will be sent over i2c.
  char *strtokIndex;
  char *slice_module_name = NULL;
  char *slice_module_command = NULL;
  char *module_port = NULL;
  char *module_command_value = NULL;
  char limit[] = "/";
  char packet_copy[256];
  II_command *final;
  Module_info *ii_module;

/*

Check max port number with the help of module_info !!

/////////////
/////////////
/////////////
/////////////
/////////////
/////////////
*/


  // Make a copy of the input string
  strncpy(packet_copy, packet, sizeof(packet_copy));

  if (packet_copy[0] == '/') {
    slice_module_name = strtok_r(packet_copy, limit, &strtokIndex);
    if (slice_module_name != NULL) {
        ii_module = retrieve_module(slice_module_name);

        slice_module_command = strtok_r(NULL, limit, &strtokIndex);
        final = retrieve_cmd(ii_module, slice_module_command);
        if(final == NULL) {
          // handle error here
          printf("Command unknown\n");
          return;

        } else {
          printf("Command received is %s\n", final->name);
         module_port = strtok_r(NULL, limit, &strtokIndex);
          if (module_port != NULL) { 
              int i = atoi(module_port);
              if (i == 0 && module_port[0] != '0') {
                  printf("module port is not a number\n");
                  return;
              }
              printf("module port is %d\n", i);
          } else {
              printf("no module port provided\n");
              return;
          }
        int arg_count = 0;
          for (int i = 0; i < 2; i++) {
            if (final->args[i].name[0] != '\0') {
              arg_count++;
            }
          }
          if (arg_count > 1) {
            module_command_value = strtok_r(NULL, limit, &strtokIndex);
            if (module_command_value != NULL) {
              char *endptr;
              long int command_value = strtol(module_command_value, &endptr, 10);
              if (endptr == module_command_value || *endptr != '\0' || command_value < 0 || command_value > 32767) {
                printf("Invalid command value: %s\n", module_command_value);
                return;
              }
              printf("Command value received is %ld\n", command_value);
            } else {
              printf("Missing value\n");
              return;
            }
          }
        }
    }
  }
}


// UDP server callback
static void udp_server_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    // Handle the received data here
    parseData((char*)p->payload);
    // Free the received buffer
    pbuf_free(p);
}

int main() {

    /* Initialize i2c */ 
    i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    stdio_init_all();

    cyw43_arch_init();

    const char *ap_name = "hans";
    const char *password = "password";

    cyw43_arch_enable_ap_mode(ap_name, password, CYW43_AUTH_WPA2_AES_PSK);

    ip4_addr_t gw, mask;
    IP4_ADDR(&gw, 192, 168, 4, 1);
    IP4_ADDR(&mask, 255, 255, 255, 0);
 
    // Start the dhcp server
    dhcp_server_t dhcp_server;
    dhcp_server_init(&dhcp_server, &gw, &mask);

    // Create a new UDP PCB (Protocol Control Block)
    struct udp_pcb *pcb = udp_new();
    // Bind the UDP PCB to the server port
    udp_bind(pcb, IP_ADDR_ANY, UDP_PORT);
    // Set the receive callback function for the UDP PCB
    udp_recv(pcb, udp_server_recv, NULL);

    // Action!
    while (1)
    {
        /* listen to OSC over UDP */
      
    }

    // And that's a wrap!
    udp_remove(pcb);
    cyw43_arch_deinit();

    return 0;
}

