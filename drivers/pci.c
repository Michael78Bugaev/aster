#include <drv/pci.h>
#include <string.h>
#include <stdio.h>
#include <config.h>
#include <io/iotools.h>
#include <stdbool.h>

uint32_t pci_size_map[100];
pci_dev_t dev_zero= {0};
bool devyes = false;

void initialize_and_print_pci_devices();
uint32_t get_device_class(pci_dev_t dev);
uint32_t get_device_subclass(pci_dev_t dev);

/*
 * Given a pci device(32-bit vars containing info about bus, device number, and function number), a field(what u want to read from the config space)
 * Read it for me !
 * */
uint32_t pci_read(pci_dev_t dev, uint32_t field) {
	// Only most significant 6 bits of the field
	dev.field_num = (field & 0xFC) >> 2;
	dev.enable = 1;
	outportl(PCI_CONFIG_ADDRESS, dev.bits);

	// What size is this field supposed to be ?
	uint32_t size = pci_size_map[field];
	if(size == 1) {
		// Get the first byte only, since it's in little endian, it's actually the 3rd byte
		uint8_t t =port_byte_in(PCI_CONFIG_DATA + (field & 3));
		return t;
	}
	else if(size == 2) {
		uint16_t t = inports(PCI_CONFIG_DATA + (field & 2));
		return t;
	}
	else if(size == 4){
		// Read entire 4 bytes
		uint32_t t = inportl(PCI_CONFIG_DATA);
		return t;
	}
	return 0xffff;
}

/*
 * Write pci field
 * */
void pci_write(pci_dev_t dev, uint32_t field, uint32_t value) {
	dev.field_num = (field & 0xFC) >> 2;
	dev.enable = 1;
	// Tell where we want to write
	outportl(PCI_CONFIG_ADDRESS, dev.bits);
	// Value to write
	outportl(PCI_CONFIG_DATA, value);
}

/*
 * Get device type (i.e, is it a bridge, ide controller ? mouse controller? etc)
 * */
uint32_t get_device_type(pci_dev_t dev) {
	uint32_t t = pci_read(dev, PCI_CLASS) << 8;
	return t | pci_read(dev, PCI_SUBCLASS);
}

uint32_t get_device_class(pci_dev_t dev)
{
    return pci_read(dev, PCI_CLASS);
}
uint32_t get_device_subclass(pci_dev_t dev)
{
    return pci_read(dev, PCI_SUBCLASS);
}

/*
 * Get secondary bus from a PCI bridge device
 * */
uint32_t get_secondary_bus(pci_dev_t dev) {
	return pci_read(dev, PCI_SECONDARY_BUS);
}

/*
 * Is current device an end point ? PCI_HEADER_TYPE 0 is end point
 * */
uint32_t pci_reach_end(pci_dev_t dev) {
	uint32_t t = pci_read(dev, PCI_HEADER_TYPE);
	return !t;
}

/*
 * The following three functions are basically doing recursion, enumerating each and every device connected to pci
 * We start with the primary bus 0, which has 8 function, each of the function is actually a bus
 * Then, each bus can have 8 devices connected to it, each device can have 8 functions
 * When we gets to enumerate the function, check if the vendor id and device id match, if it does, we've found our device !
 **/

/*
 * Scan function
 * */
pci_dev_t pci_scan_function(uint16_t vendor_id, uint16_t device_id, uint32_t bus, uint32_t device, uint32_t function, int device_type) {
	pci_dev_t dev = {0};
	dev.bus_num = bus;
	dev.device_num = device;
	dev.function_num = function;
	// If it's a PCI Bridge device, get the bus it's connected to and keep searching
	if(get_device_type(dev) == PCI_TYPE_BRIDGE) {
		pci_scan_bus(vendor_id, device_id, get_secondary_bus(dev), device_type);
	}
	// If type matches, we've found the device, just return it
	if(device_type == -1 || device_type == get_device_type(dev)) {
		uint32_t devid  = pci_read(dev, PCI_DEVICE_ID);
		uint32_t vendid = pci_read(dev, PCI_VENDOR_ID);
		if(devid == device_id && vendor_id == vendid)
			return dev;
	}
	return dev_zero;
}

/*
 * Scan device
 * */
pci_dev_t pci_scan_device(uint16_t vendor_id, uint16_t device_id, uint32_t bus, uint32_t device, int device_type) {
	pci_dev_t dev = {0};
	dev.bus_num = bus;
	dev.device_num = device;

	if(pci_read(dev,PCI_VENDOR_ID) == PCI_NONE)
		return dev_zero;

	pci_dev_t t = pci_scan_function(vendor_id, device_id, bus, device, 0, device_type);
	if(t.bits)
		return t;

	if(pci_reach_end(dev))
		return dev_zero;

	for(int function = 1; function < FUNCTION_PER_DEVICE; function++) {
		if(pci_read(dev,PCI_VENDOR_ID) != PCI_NONE) {
			t = pci_scan_function(vendor_id, device_id, bus, device, function, device_type);
			if(t.bits)
				return t;
		}
	}
	return dev_zero;
}
/*
 * Scan bus
 * */
pci_dev_t pci_scan_bus(uint16_t vendor_id, uint16_t device_id, uint32_t bus, int device_type) {
	for(int device = 0; device < DEVICE_PER_BUS; device++) {
		pci_dev_t t = pci_scan_device(vendor_id, device_id, bus, device, device_type);
		if(t.bits)
			return t;
	}
	return dev_zero;
}

/*
 * Device driver use this function to get its device object(given unique vendor id and device id)
 * */
pci_dev_t pci_get_device(uint16_t vendor_id, uint16_t device_id, int device_type) {

	pci_dev_t t = pci_scan_bus(vendor_id, device_id, 0, device_type);
	if(t.bits)
		return t;

	// Handle multiple pci host controllers

	if(pci_reach_end(dev_zero)) {
		INFO("PCI det device failed");
	}
	for(int function = 1; function < FUNCTION_PER_DEVICE; function++) {
		pci_dev_t dev = {0};
		dev.function_num = function;

		if(pci_read(dev, PCI_VENDOR_ID) == PCI_NONE)
			break;
		t = pci_scan_bus(vendor_id, device_id, function, device_type);
		if(t.bits)
			return t;
	}
	return dev_zero;
}

/*
 * PCI Init, filling size for each field in config space
 * */
void pci_init() {
	// Init size map
	pci_size_map[PCI_VENDOR_ID] =	2;
	pci_size_map[PCI_DEVICE_ID] =	2;
	pci_size_map[PCI_COMMAND]	=	2;
	pci_size_map[PCI_STATUS]	=	2;
	pci_size_map[PCI_SUBCLASS]	=	1;
	pci_size_map[PCI_CLASS]		=	1;
	pci_size_map[PCI_CACHE_LINE_SIZE]	= 1;
	pci_size_map[PCI_LATENCY_TIMER]		= 1;
	pci_size_map[PCI_HEADER_TYPE] = 1;
	pci_size_map[PCI_BIST] = 1;
	pci_size_map[PCI_BAR0] = 4;
	pci_size_map[PCI_BAR1] = 4;
	pci_size_map[PCI_BAR2] = 4;
	pci_size_map[PCI_BAR3] = 4;
	pci_size_map[PCI_BAR4] = 4;
	pci_size_map[PCI_BAR5] = 4;
	pci_size_map[PCI_INTERRUPT_LINE]	= 1;
	pci_size_map[PCI_SECONDARY_BUS]		= 1;
    initialize_and_print_pci_devices();
}

// Таблица производителей
vendor_info_t vendors[] = {
    {0x8086, "Intel"},
    {0x10DE, "NVIDIA"},
    {0x1022, "AMD"},
    {0x1B36, "Realtek"},
    // Добавь сюда еще производителей, если надо
};

// Таблица устройств
device_info_t devices[] = {
    {0x8086, 0x1234, "Intel Device 1234"},
    {0x10DE, 0x1C82, "NVIDIA GeForce GTX 1080"},
    {0x1022, 0x2000, "AMD Device 2000"},
    // Добавь сюда еще устройства, если надо
};

// Функция для поиска названия производителя по vendor_id
const char* get_vendor_name(uint16_t vendor_id) {
    for (int i = 0; i < sizeof(vendors) / sizeof(vendor_info_t); i++) {
        if (vendors[i].vendor_id == vendor_id) {
            return vendors[i].name;
        }
    }
    return "Unknown Vendor";
}

// Функция для поиска названия устройства по vendor_id и device_id
const char* get_device_name(uint16_t vendor_id, uint16_t device_id) {
    switch (vendor_id)
    {
        case 0x15AD: // Vmware
            switch (device_id)
            {
                case 0x07A0:
                    if (!devyes)
                    {
                        return "Vmware PCI Express Root Port";
                        devyes = true;
                    }
                    break;
                case 0x07E0:
                    return "Vmware SATA AHCI controller";
                    break;
                case 0x0770:
                    return "Vmware USB2 EHCI Controller";
                    break;
                case 0x0774:
                    return "Vmware USB1.1 UHCI Controller";
                    break;
                default:
                    return "VMware Device";
                    break;
            }
            break;
        case 0x8086: // Integrated Electronics (Intel)
            switch (device_id)
            {
                case 0x1237:
                    return "Intel 440FX - 82441FX PMC [Natoma]";
                    break;
                case 0x7000:
                    return "Intel 82371SB PIIX3 ISA [Natoma/Triton II]";
                    break;
                case 0x7010:
                    return "Intel 82371SB PIIX3 IDE [Natoma/Triton II]";
                    break;
                case 0x7113:
                    return "Intel 82371AB/EB/MB PIIX4 ACPI";
                    break;
                case 0x100E:
                    return "Intel 82540EM Gigabit Ethernet Controller";
                    break;
                case 0xA000:
                    return "Intel Atom Processor D4xx/D5xx/N4xx/N5xx DMI Bridge";
                    break;
                case 0xA001:
                    return "Intel Atom Processor D4xx/D5xx/N4xx/N5xx Integrated Graphics Controller";
                    break;
                case 0x27D8:
                    return "Intel NM10/ICH7 Family High Definition Audio Controller";
                    break;
                case 0x27D0:
                    return "Intel NM10/ICH7 Family PCI Express Port 1";
                    break;
                case 0x27D2:
                    return "Intel NM10/ICH7 Family PCI Express Port 2";
                    break;
                case 0x27D4:
                    return "Intel NM10/ICH7 Family PCI Express Port 3";
                    break;
                case 0x27D6:
                    return "Intel NM10/ICH7 Family PCI Express Port 4";
                    break;
                case 0x27C8:
                    return "Intel NM10/ICH7 Family USB UHCI Controller #1";
                    break;
                case 0x27C9:
                    return "Intel NM10/ICH7 Family USB UHCI Controller #2";
                    break;
                case 0x27CA:
                    return "Intel NM10/ICH7 Family USB UHCI Controller #3";
                    break;
                case 0x27CB:
                    return "Intel NM10/ICH7 Family USB UHCI Controller #4";
                    break;
                case 0x27CC:
                    return "Intel NM10/ICH7 Family USB2 EHCI Controller";
                    break;
                case 0x2448:
                    return "Intel 82801 Mobile PCI Bridge";
                    break;
                case 0x27BC:
                    return "Intel NM10 Family LPC Controller";
                    break;
                case 0x27C1:
                    return "Intel NM10/ICH7 Family SATA Controller [AHCI mode]";
                    break;
                case 0x27DA:
                    return "Intel NM10/ICH7 Family SMBus Controller";
                    break;
                
                default:
                    return "Intel Device";
                    break;
            }
            break;
        case 0x1002: // AMD/ATI
            switch (device_id)
            {
                case 0x4391:
                    return "AMD/ATI SB7x0/SB8x0/SB9x0 SATA Controller [AHCI mode]";
                    break;
                case 0x4398:
                    return "AMD/ATI SB7x0 USB OHCI1 Controller";
                    break;
                case 0x4396:
                    return "AMD/ATI SB7x0/SB8x0/SB9x0 USB EHCI Controller";
                    break;
                case 0x4397:
                    return "AMD/ATI SB7x0/SB8x0/SB9x0 USB OHCI0 Controller";
                    break;
                case 0x4385:
                    return "AMD/ATI SBx00 SMBus Controller";
                    break;
                case 0x4383:
                    return "AMD/ATI SBx00 Azalia (Intel HDA)";
                    break;
                case 0x439D:
                    return "AMD/ATI SB7x0/SB8x0/SB9x0 LPC host controller";
                    break;
                case 0x4384:
                    return "SBx00 PCI to PCI Bridge";
                    break;
                case 0x4399:
                    return "AMD/ATI SB7x0/SB8x0/SB9x0 USB OHCI2 Controller";
                    break;
                case 0x9600:
                    return "AMD RS780 Host Bridge";
                    break;
                case 0x9602:
                    return "AMD RS780/RS880 PCI to PCI bridge (int gfx)";
                    break;
                case 0x9604:
                    return "AMD RS780/RS880 PCI to PCI bridge (PCIE port 0)";
                    break;
                case 0x9605:
                    return "AMD RS780/RS880 PCI to PCI bridge (PCIE port 1)";
                    break;
                case 0x9607:
                    return "AMD RS780/RS880 PCI to PCI bridge (PCIE port 3)";
                    break;
                case 0x9608:
                    return "AMD RS780/RS880 PCI to PCI bridge (PCIE port 4)";
                    break;
                default:
                    return "AMD Device";
                    break;
            }
            break;
        case 0x1022: // Advanced Micro Controllers (AMD)
            switch (device_id)
            {
                case 0x2000:
                    return "AMD 79c970 (PCnet32 LANCE)";
                    break;
                case 0x1300:
                    return "AMD Family CPU HyperTransport Configuration";
                    break;
                case 0x1301:
                    return "AMD Family 11h Processor Address Map";
                    break;
                case 0x1302:
                    return "AMD Family 11h Processor DRAM Controller";
                    break;
                case 0x1303:
                    return "AMD Family 11h Processor Miscellaneous Control";
                    break;
                case 0x1304:
                    return "AMD Family 11h Processor Link Control";
                    break;
                
                default:
                    return "AMD Device";
                    break;
            }
            break;
        
        case 0x14E4: // Broadcom
            switch (device_id)
            {
                case 0x1693:
                    return "NetLink BCM5787M Gigabit Ethernet PCI Express";
                    break;
            
                default:
                    return "Broadcom Device";
                    break;
            }
            break;
            case 0x11C1: // LSI (Broadcom)
                switch (device_id)
                {
                    case 0x5811:
                        return "LSI (Broadcom) NetLink BCM5787M Gigabit Ethernet PCI Express";
                        break; 
                    default:
                        return "LSI (Broadcom) Device";
                        break;
                }
            break;
        default:
            return "Unknown Device";
            break;
    }
}

void initialize_and_print_pci_devices() {
    pci_dev_t dev;
    uint16_t vendor_id, device_id, class_id;

    // Сканируем все шины, устройства и функции
    for (uint32_t bus = 0; bus < 256; bus++) {
        for (uint32_t device = 0; device < 32; device++) {
            for (uint32_t function = 0; function < 8; function++) {
                dev.bus_num = bus;
                dev.device_num = device;
                dev.function_num = function;

                // Считываем vendor ID
                vendor_id = pci_read(dev, PCI_VENDOR_ID);
                
                // Проверяем, существует ли устройство
                if (vendor_id == 0x00FF || vendor_id == 0xFFFF) {
                    continue; // Устройство не существует, переходим к следующему
                }

                // Считываем device ID
                device_id = pci_read(dev, PCI_DEVICE_ID);
                
                // Проверяем, действительно ли устройство активно
                if (device_id == 0x00FF || vendor_id == 0xFFFF) {
                    continue; // Если device ID невалиден, пропускаем
                }

                // Считываем class ID
                class_id = get_device_class(dev);
                uint32_t subclass = get_device_subclass(dev);
                const char* device_name = get_device_name(vendor_id, device_id);

                // Вывод информации об устройстве
                INFO("pci [%4x:%4x] class: 0x%2x subclass: 0x%2x", vendor_id, device_id, class_id, subclass);
            }
        }
    }
}