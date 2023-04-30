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

/* Debug */
#define DEBUG 1

/* I2c pins on Pico */
#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9
#define I2C_PORT i2c0

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

II_command* retrieve_cmd(Module_info* module_info, char* module_command) {
  int i;
  for(i = 0; i < module_info->number_of_cmd; i++) {
    if(strcmp(module_info->cmd_set[i].name, module_command) == 0) {
      return &module_info->cmd_set[i];
    }
  }
  return NULL;
}

int retrieve_module_address(Module_info *module_info, int module_port) {
    if (module_port < 0 || module_port >= module_info->max_ports) {
        printf("Invalid module port: %d\n", module_port);
        return 0;
    }

    if (module_info == &er301_info) {
        int address = module_info->addresses[module_port / 100];
        #ifdef DEBUG
            printf("Retrieved ER-301 module address: %d\n", address);
        #endif
        return address;
    }
    else if (module_info == &txo_info) {
        int address = module_info->addresses[module_port / 8];
        #ifdef DEBUG
            printf("Retrieved TXO module address: %d\n", address);
        #endif
        return address;
    }
    else {
        // add support for more modules here
        return 0;
    }
}

void send_i2c_msg(Module_info* module_info, II_command* ii_cmd, int module_port, long int cmd_value) {
    int address = retrieve_module_address(module_info, module_port);

    // Construct the I2C message buffer: the command may require a value
    uint8_t i2c_buf[6];
    i2c_buf[0] = address;
    i2c_buf[1] = ii_cmd->command_number;
    i2c_buf[2] = module_port;
    if (ii_cmd->arg_count > 1) {
        i2c_buf[3] = (cmd_value >> 8) & 0xFF;  // high byte
        i2c_buf[4] = cmd_value & 0xFF;         // low byte
        size_t buf_len = 5;
    } else {
        size_t buf_len = 3;
    }

    // Send the I2C message

    // Print debug information
    char buffer[256];
    sprintf(buffer, "Module: %s, Command: %x, Port: %d, Value: %ld\n", module_info->name, ii_cmd->command_number, module_port, cmd_value);
    printf(buffer);
}

void parseData(char packet[]) {
    const char *delim = "/";
    char packet_copy[256];
    char buffer[256];
    strncpy(packet_copy, packet, sizeof(packet_copy));

    Module_info *module_info = NULL;
    II_command *ii_cmd = NULL;
    int module_port = -1;

    char *module_name = strtok(packet_copy, delim);
    if (module_name == NULL) {
        printf("Invalid module name: %s\n", packet);
        return;
    }

    module_info = retrieve_module(module_name);
    if (module_info == NULL) {
        printf("Module not found: %s\n", module_name);
        return;
    }

    char *cmd_name = strtok(NULL, delim);
    if (cmd_name == NULL) {
        printf("Missing command name\n");
        return;
    }

    ii_cmd = retrieve_cmd(module_info, cmd_name);
    if (ii_cmd == NULL) {
        printf("Command not found: %s\n", cmd_name);
        return;
    }

    char *module_port_str = strtok(NULL, delim);
    if (module_port_str == NULL) {
        printf("Missing module port\n");
        return;
    }

    module_port = atoi(module_port_str);
    if (module_port == 0 && module_port_str[0] != '0') {
        printf("Invalid module port: %s\n", module_port_str);
        return;
    }

    if (module_port >= module_info->max_ports) {
        printf("Module port out of range: %d\n", module_port);
        return;
    }

    int address = retrieve_module_address(module_info, module_port);

    if (ii_cmd->arg_count > 1) {
        char *cmd_value_str = strtok(NULL, delim);
        if (cmd_value_str == NULL) {
            printf("Missing command value\n");
            return;
        }

        char *endptr;
        long int cmd_value = strtol(cmd_value_str, &endptr, 10);
        if (endptr == cmd_value_str || *endptr != '\0' || cmd_value < 0 || cmd_value > 32767) {
            printf("Invalid command value: %s\n", cmd_value_str);
            return;
        }

        // Construct the I2C message buffer: the command requires a value


    } else {

        // [device address, command number, argument byte 1, argument byte 2]

        // Construct the I2C message buffer: the command does not require a value
    }

    #ifdef DEBUG
        printf("%s", buffer);
    #endif
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

