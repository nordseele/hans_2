#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include <string.h>
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"
#include "ii.h"
#include <stdarg.h>
#include "dhcpserver.h"
#include "dnsserver.h"
#include <lwip/udp.h>
#include "lwip/pbuf.h"
#include "lwip/sockets.h"
#include "midi_func.h"
#include "helpers.h"

/* Debug */
#define DEBUG 1

/* I2c pins on Pico */
#define I2C_SDA_PIN 12
#define I2C_SCL_PIN 13
#define I2C_PORT i2c0

// Define the UDP port to listen on
#define UDP_PORT 9301

/* UART (console) */
#define UART_ID uart1
#define BAUD_RATE 115200
#define UART_TX_PIN 8
#define UART_RX_PIN 9

/* MIDI */
#define UART_ID_2 uart0
#define BAUD_RATE_2 31250
#define UART_TX_2_PIN 0
#define UART_RX_2_PIN 1

// Define the maximum length of incoming OSC messages
#define MAX_OSC_LEN 256


Module_info* retrieve_module(char* slice_module_name) {
  if(strcmp(slice_module_name, "er301") == 0) {
    return &er301_info;
  } else if(strcmp(slice_module_name, "txo") == 0) {
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
        printy("Invalid module port: %d\n", module_port);
        return 0;
    }

    if (module_info == &er301_info) {
        int address = module_info->addresses[module_port / 100];
        return address;
    }
    else if (module_info == &txo_info) {
        int address = module_info->addresses[module_port / 8];
        return address;
    }
    else {
        // add support for more modules here
        return 0;
    }
}

void send_i2c_message(Module_info* module_info, II_command* ii_cmd, int module_port, long int cmd_value) {
    int address = retrieve_module_address(module_info, module_port);

    // Define the I2C message buffer and buffer length variables
    uint8_t i2c_buf[6];
    size_t buf_len;

    if (ii_cmd->arg_count > 1) {
        // The command requires a value
        if (cmd_value < 0 || cmd_value > 32767) {
            printy("Invalid command value: %ld\n", cmd_value);
            return;
        }

        // Construct the I2C message buffer with command value
        i2c_buf[0] = ii_cmd->command_number;
        i2c_buf[1] = module_port;
        i2c_buf[2] = (cmd_value >> 8) & 0xFF;  // high byte
        i2c_buf[3] = cmd_value & 0xFF;         // low byte
        buf_len = 4;
    } else {
        // The command does not require a value
        i2c_buf[0] = ii_cmd->command_number;
        i2c_buf[1] = module_port;
        buf_len = 2;
    }

    #ifdef DEBUG
        // Print debug information
        char buffer[256];
        if (buf_len == 4) {
            sprintf(buffer, "Module: %s, Command: %x, Port: %d, Value: %ld\n", module_info->name, ii_cmd->command_number, module_port, cmd_value);
        } else {
            sprintf(buffer, "Module: %s, Command: %x, Port: %d\n", module_info->name, ii_cmd->command_number, module_port);
        }
        printy(buffer);
    #endif

    // Send the I2C message
    i2c_write_blocking(I2C_PORT, address, i2c_buf, buf_len, false);
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
        printy("Invalid module name: %s\n", packet);
        return;
    }

    module_info = retrieve_module(module_name);
    if (module_info == NULL) {
        printy("Module not found: %s\n", module_name);
        return;
    }

    char *cmd_name = strtok(NULL, delim);
    if (cmd_name == NULL) {
        printy("Missing command name\n");
        return;
    }

    ii_cmd = retrieve_cmd(module_info, cmd_name);
    if (ii_cmd == NULL) {
        printy("Command not found: %s\n", cmd_name);
        return;
    }

    char *module_port_str = strtok(NULL, delim);
    if (module_port_str == NULL) {
        printy("Missing module port\n");
        return;
    }

    module_port = atoi(module_port_str);
    if (module_port == 0 && module_port_str[0] != '0') {
        printy("Invalid module port: %s\n", module_port_str);
        return;
    }

    if (module_port >= module_info->max_ports) {
        printy("Module port out of range: %d\n", module_port);
        return;
    }

    // Check if the command requires a value
    bool has_value = (ii_cmd->arg_count > 1);

    // Call the new function with the necessary parameters
    if (has_value) {
        // Extract the command value from the packet
        char *cmd_value_str = strtok(NULL, delim);
        if (cmd_value_str == NULL) {
            printy("Missing command value\n");
            return;
        }

        char *endptr;
        long int cmd_value = strtol(cmd_value_str, &endptr, 10);
        if (endptr == cmd_value_str || *endptr != '\0' || cmd_value < 0 || cmd_value > 32767) {
            printy("Invalid command value: %s\n", cmd_value_str);
            return;
        }

        send_i2c_message(module_info, ii_cmd, module_port, cmd_value);
    } else {
        send_i2c_message(module_info, ii_cmd, module_port, 0);
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

    /* Initialize UART (console on UART 1)*/
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    /* Initialize UART (MIDI on UART 0)*/
    uart_init(UART_ID_2, BAUD_RATE_2);
    gpio_set_function(UART_TX_2_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_2_PIN, GPIO_FUNC_UART);
    uart_set_hw_flow(UART_ID_2, false, false);


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

    printy("\rstarting hans OSC\n");
    
    // Action!
    while (1)
    {

        /*  
            listen to OSC over UDP
            ****************************
            ****** DONE ELSEWHERE ******
            ****************************
            ****************************
        */


        /* listen to MIDI over UART */
        while (uart_is_readable(UART_ID_2)) {
            uint8_t status = uart_getc(UART_ID_2);
            uint8_t data1 = uart_getc(UART_ID_2);
            uint8_t data2 = uart_getc(UART_ID_2);

            // Create a temporary buffer to hold the MIDI bytes
            uint8_t midi_bytes[3] = { status, data1, data2 };

            // Call the MIDI message handling function
            parse_midi_command(midi_bytes);
        }

    }

    // And that's a wrap!
    udp_remove(pcb);
    cyw43_arch_deinit();

    return 0;
}

