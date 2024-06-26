#include "../lib/disco.h"
#include "../lib/structs.h"
#include "../lib/scanner.h"

#include <vector>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <algorithm>

using namespace std;

scanner scan;
int startValue;

Disk :: Disk(){}

void Disk::mkdisk(vector<string> tokens){
    string size = "";
    string u = "";
    string path = "";
    string f = "";
    bool error = false;
    for(string token:tokens){
        string tk = token.substr(0, token.find("=")); // -f=b
        token.erase(0,tk.length()+1); // b
        if(scan.compare(tk, "f")){
            if(f.empty()){
                f = token; // f = b
            }else{
                scan.errores("MKDISK", "El parametro F ya fue ingresado en el comando"+tk);
            }
        }else if(scan.compare(tk, "s")){
            if (size.empty())
            {
                size = token;
            }else{
                scan.errores("MKDISK","parametro SIZE repetido en el comando"+tk);
            }
        }else if (scan.compare(tk, "u"))
        {
            if (u.empty())
            {
                u = token;
            }else{
                scan.errores("MKDISK","parametro U repetido en el comando"+tk);
            }
        }else if (scan.compare(tk, "path"))
        {
            if (path.empty())
            {   
                if (token.substr(0, 1) == "\"")
                {
                    token = token.substr(1, token.length()-2 );
                }
                path = token;
            }else{
                scan.errores("MKDISK","parametro PATH repetido en el comando"+tk);
            }    
        }else{
            scan.errores("MKDISK","no se esperaba el parametro "+tk);
            error = true;
            break;
        }
    }
    if (error){
        return;
    }

    if (f.empty())
    {
        f = "BF";
    }
    if (u.empty())
    {
        u = "M";
    }

    if (path.empty() && size.empty())
    {
        scan.errores("MKDISK", "se requiere parametro Path y Size para este comando");
    }else if(path.empty()){
        scan.errores("MKDISK","se requiere parametro Path para este comando");
    }else if (size.empty())
    {
        scan.errores("MKDISK","se requiere parametro Size para este comando");
    }else if (!scan.compare(f,"BF") && !scan.compare(f,"FF") && !scan.compare(f,"WF"))
    {
        scan.errores("MKDISK","valores de parametro F no esperados");
    }else if (!scan.compare(u,"k") && !scan.compare(u,"m"))
    {
        scan.errores("MKDISK","valores de parametro U no esperados");
    }else{
        makeDisk(size,f,u,path);
    }  
}

// Crear funcion makeDisk
void Disk::makeDisk(string s, string f, string u, string path){\
    Structs::MBR disco; 
    try{
        int size = stoi(s); // stoi = string to int
        if (size <=0){
            scan.errores("MKDISK","Size debe ser mayor a 0");
        }
        if(scan.compare(u,"M")){
            size = size * 1024 * 1024;
        }
        if(scan.compare(u,"K")){
            size = size * 1024;
        }
        f = f.substr(0,1); // BF -> B
        disco.mbr_tamano = size;
        disco.mbr_fecha_creacion = time(nullptr);
        disco.disk_fit = toupper(f[0]);
        disco.mbr_disk_signature = rand() % 9999 + 100;

        FILE *file = fopen(path.c_str(),"r"); // c_str() = convertir string a char
        if(file != NULL){
            scan.errores("MKDISK","El disco ya existe");
            fclose(file);
            return;
        }

        disco.mbr_Partition_1 = Structs::Partition();
        disco.mbr_Partition_2 = Structs::Partition();
        disco.mbr_Partition_3 = Structs::Partition();
        disco.mbr_Partition_4 = Structs::Partition();

        string path2 = path;
        if(path.substr(0,1) == "\""){
            path2 = path.substr(1,path.length()-2);
        };

        if(!scan.compare(path.substr(path.find_last_of(".") + 1), "dsk")){
            scan.errores("MKDISK","El disco debe ser de tipo .dsk");
            return;
        }

        try{
            FILE *file = fopen(path.c_str(), "w+b");
            if(file!=NULL){
                fwrite("\0", 1, 1, file);
                fseek(file, size-1, SEEK_SET);
                fwrite("\0", 1, 1, file);
                rewind(file);
                fwrite(&disco, sizeof(Structs::MBR), 1, file);
                fclose(file);
            }else{
                string comando1 = "mkdir -p \""+ path + "\"";
                string comando2 = "rmdir \""+ path + "\"";
                system(comando1.c_str());
                system(comando2.c_str());
                FILE *file = fopen(path.c_str(), "w+b");
                fwrite("\0",1,1,file);
                fseek(file, size - 1, SEEK_SET);
                fwrite("\0", 1, 1, file);
                rewind(file);
                fwrite(&disco, sizeof(Structs::MBR),1, file);
                fclose(file);
            }

            FILE *imprimir = fopen(path.c_str(), "r");
            if(imprimir!=NULL){
                Structs::MBR discoI;
                fseek(imprimir, 0, SEEK_SET);
                fread(&discoI,sizeof(Structs::MBR), 1,imprimir);
                struct tm *tm;
                tm = localtime(&discoI.mbr_fecha_creacion);
                char mostrar_fecha [20];
                strftime(mostrar_fecha, 20, "%Y/%m/%d %H:%M:%S", tm);                
                scan.respuesta("MKDISK","   Disco creado exitosamente");
                std::cout << "********Nuevo Disco********" << std::endl;
                std::cout << "Size:  "<< discoI.mbr_tamano << std::endl;
                std::cout << "Fecha:  "<< mostrar_fecha << std::endl;
                std::cout << "Fit:  "<< discoI.disk_fit << std::endl;
                std::cout << "Disk_Signature:  "<< discoI.mbr_disk_signature << std::endl;
                cout << "Bits del MBR:  " << sizeof(Structs::MBR) << endl;
                std::cout << "Path:  "<< path2 << std::endl;
            }
            fclose(imprimir);

        }catch(const exception& e){
            scan.errores("MKDISK","Error al crear el disco");
        }
    }catch(invalid_argument &e){
        scan.errores("MKDISK","Size debe ser un entero");
    }

}
// Borrar disco mkdisk -u=m -s=10 -path="/home/s s.dsk"
void Disk::rmdisk(vector<string> context){
    string path = "";
    if (context.size()==0)
    {
        scan.errores("RMDISK","Se esperaba el path para completar la acción");
    }
    
    for (string token:context)
    {
        string tk = token.substr(0, token.find("="));
        token.erase(0,tk.length()+1);
        if (scan.compare(tk, "path"))
        {
            path= token;
        }else{
            path = "";
            scan.errores("RMDISK","No se reconoce este elemento "+tk);
            break;
        }
    }
    if (!path.empty())
    {
        if (path.substr(0, 1) == "\"")
        {
            path = path.substr(1, path.length() - 2);
        }
        try
        {
            FILE *file = fopen(path.c_str(), "r");
            
            if (file != NULL)
            {
                if(!scan.compare(path.substr(path.find_last_of(".") + 1),"dsk")){
                    scan.errores("RMDISK", "Extensión de archivo no valida debe ser tipo dsk");
                    return;
                }
                fclose(file);
                if (scan.confirmar("¿Desea eliminar el archivo?"))
                {
                    if (remove(path.c_str()) == 0)
                    {
                        scan.respuesta("RMDISK","Disco eliminado correctamente");
                        return;
                    }
                }else{
                    scan.respuesta("RMDISK","Operación cancelada");
                    return;
                }
            }
            scan.errores("RMDISK", "El disco que desea eliminar no existe en la ruta indicada");
        }
        catch(const std::exception& e)
        {
            scan.errores("RMDISK","Error al intentar eliminar el disco");
        }
        
    }
    
}

void Disk::fdisk(vector<string> context)
{
    bool dtl = false;
    for (string current: context)
    {
        string id = current.substr(0,current.find("="));
        current.erase(0,id.length()+1);
        if(current.substr(0,1)=="\"")
        {
            current = current.substr(1,current.length()-2);
        }
        if(scan.compare(id,"delete"))
        {
            dtl = true;
        }
    }

    if(dtl)
    {
        fdisk_d(context);
    }
    else
    {
        fdisk_c(context);
    }
}

void Disk::fdisk_c(vector<string> context){
    vector<string> required = {"s", "path", "name"};
    string size;
    string u = "k";
    string path;
    string type = "P";
    string f = "WF";
    string name;
    string add;

    for(auto current: context){
        string id = current.substr(0, current.find("="));
        current.erase(0, id.length() + 1);
        if(current.substr(0, 1) == "\""){
            current = current.substr(1, current.length() - 2);
        }

        if (scan.compare(id, "s"))
        {
            if(count(required.begin(), required.end(), id)){
                auto itr = find(required.begin(), required.end(), id);
                required.erase(itr);
                size = current;
            }
        }else if (scan.compare(id, "u")){
            u = current;
        }else if(scan.compare(id, "path")){
            if(count(required.begin(), required.end(), id)){
                auto itr = find(required.begin(), required.end(), id);
                required.erase(itr);
                path = current;
            }
        }else if (scan.compare(id, "t")){
            type = current;
        }else if (scan.compare(id, "f")){
            f = current;
        }else if (scan.compare(id, "name")){
            if(count(required.begin(), required.end(), id)){
                auto itr = find(required.begin(), required.end(), id);
                required.erase(itr);
                name = current;
            }
        }else if (scan.compare(id, "add")){
            add = current;
            if (count(required.begin(), required.end(), "s")) {
                auto itr = find(required.begin(), required.end(), "s");
                required.erase(itr);
                size = current;
            }

        }else{
            scan.errores("FDISK","No se reconoce el parametro "+id);
        }
    }

    if(!required.empty()){
        string faltantes = "";
        for(string faltante:required)
        {
            faltantes += faltante+" ";
        }
        faltantes = "Faltan parametros obligatorios para completar la acción: " + faltantes;
        scan.errores("FDISK",faltantes);
        return;
    }else{
        if(add.empty())
        {
            cout << "Generar particion" << endl;
            cout << "Size: " << size << endl;
            cout << "U: " << u << endl;
            cout << "Path: " << path << endl;
            cout << "Type: " << type << endl;
            cout << "F: " << f << endl;
            cout << "Name: " << name << endl;
            cout << "Add: " << add << endl;
            generatepartition(size, u, path, type, f, name, add);
        }else{
            addpartition(add, u, name, path);
        }
    }
}

void Disk::fdisk_d(vector<string> context)
{
    vector<string> required = {"delete", "path", "name"};
    string path;
    string _delete;
    string name;
    for(string current:context){
        string id = current.substr(0, current.find("="));
        current.erase(0, id.length() + 1);
        if (current.substr(0, 1) == "\"") {
                current = current.substr(1, current.length() - 2);
            }
        if(scan.compare(id,"path")){
            if(count(required.begin(), required.end(), id)){
                auto itr = find(required.begin(), required.end(), id);
                required.erase(itr);
                path = current;
            }
        }else if (scan.compare(id,"delete"))
        {
            if(count(required.begin(), required.end(), id)){
                auto itr = find(required.begin(), required.end(), id);
                required.erase(itr);
                _delete = current;
            }
        }else if(scan.compare(id,"name")){
            if(count(required.begin(), required.end(), id)){
                auto itr = find(required.begin(), required.end(), id);
                required.erase(itr);
                name = current;
            }
        }

    }

    if(!required.empty()){
        string faltantes = "";
        for(string faltante:required)
        {
            faltantes += faltante+" ";
        }
        faltantes = "Faltan parametros obligatorios para completar la acción: " + faltantes;
        scan.errores("FDISK",faltantes);
        return;
    }
    else
    {
        deletepartition( _delete, path, name);
    }
}

void Disk::generatepartition(string s, string u, string p, string t, string f, string n, string a)
{
    try
    {
        startValue = 0;
        int i = stoi(s);
        if(i<=0)
        {
            shared.handler("FDISK","-s debe ser mayor a 0");
            return;
        }
        if(shared.compare(u,"b") || shared.compare(u,"k") || shared.compare(u,"m"))
        {
            if(!shared.compare(u, "b")){
                i *= (shared.compare(u, "k") ? 1024 : 1024 * 1024);
            }
        }
        else
        {
            shared.handler("FDISK","-u debe se m , k o b");
            return;
        }
        if (p.substr(0,1)=="\"")
        {
            p = p.substr(1,p.length()-2);
        }
        if(!(shared.compare(t, "p") || shared.compare(t, "e") || shared.compare(t, "l"))){
            shared.handler("FDISK", "El tipo debe ser p, e o l");
            return;
        }
        if (!(shared.compare(f, "bf") || shared.compare(f,"ff") || shared.compare(f, "wf")))
        {
            shared.handler("FDISK", "El fit debe ser bf, ff o wf");
            return;
        }
        Structs :: MBR disco;
        FILE *file = fopen(p.c_str(), "rb+");
        if(file == NULL){
            shared.handler("FDISK", "El disco no existe");
            return;
        }else{
            rewind(file);
            fread(&disco, sizeof(Structs::MBR), 1, file);
        }
        fclose(file);

        vector<Structs::Partition> particiones = getPartitions(disco);
        vector<Transition> between;

        int used = 0;
        int ext = 0;
        int c = 1;
        int base = sizeof(disco);
        Structs::Partition extended;
        for(Structs::Partition p : particiones){
            if(p.part_status == '1'){
                Transition trn;
                trn.partition = c;
                trn.start = p.part_start;
                trn.end = p.part_start + p.part_size;

                trn.before = trn.start - base;
                base = trn.end;

                if(used != 0){
                    between.at(used-1).after = trn.start - between.at(used-1).end; 
                }
                between.push_back(trn);
                used++;

                if(p.part_type == 'e' || p.part_type == 'E'){
                    ext++;
                    extended = p;
                }
            }

            if((used == 4 && (shared.compare(t, "p")))||(used == 4 && (shared.compare(t, "e")))){
                shared.handler("FDISK", "No se pueden crear mas particiones");
                return;
            }else if(ext==1 && (shared.compare(t, "e"))){
                shared.handler("FDISK", "No se pueden crear mas particiones extendidas");
                return;
            }
            c++;
        }
        if(ext == 0 && shared.compare(t, "l")){
            shared.handler("FDISK", "No se puede crear una particion logica sin una extendida");
            return;
        }
        if(used != 0){
            between.at(between.size()-1).after = disco.mbr_tamano - between.at(between.size()-1).end;
        }

        try{
            findby(disco, n,p);
            shared.handler("FDISK", "Ya existe una particion con ese nombre");
            return;
        }catch(exception &e){}

        // Aqui se crea la particion que el usuario este mandando
        Structs::Partition newPartition;
        newPartition.part_status = '1';
        newPartition.part_size = i; 
        newPartition.part_type = toupper(t[0]); // P, E, L
        newPartition.part_fit = toupper(f[0]); // B, F, W
        strcpy(newPartition.part_name, n.c_str()); // Nombre de la particion

        if(shared.compare(t, "l")){
            logic(newPartition, extended, p);
            return;
        }

        disco = adjust(disco, newPartition, between, particiones, used);

        FILE *bfile = fopen(p.c_str(), "rb+");
        if(bfile != NULL){
            fseek(bfile, 0, SEEK_SET);
            fwrite(&disco, sizeof(Structs::MBR), 1, bfile);
            if(shared.compare(t,"e")){
                Structs::EBR ebr;
                ebr.part_start = startValue;
                fseek(bfile, startValue, SEEK_SET);
                fwrite(&ebr, sizeof(Structs::EBR), 1, bfile);
            }
            fclose(bfile);
            shared.response("FDISK", "Particion creada correctamente");
        }
    }
    catch(invalid_argument &e)
    {
        scan.errores("FDISK","-s debe ser entero");
        return;
    }
    catch(exception &e)
    {
        scan.errores("FDISK generatepartition", e.what());
        return;
    }
    
}

vector<Structs::EBR> Disk::getlogics(Structs::Partition partition, string p) {
    vector<Structs::EBR> ebrs;

    FILE *file = fopen(p.c_str(), "rb+");
    rewind(file);
    Structs::EBR tmp;
    fseek(file, partition.part_start, SEEK_SET);
    fread(&tmp, sizeof(Structs::EBR), 1, file);
    do {
        if (!(tmp.part_status == '0' && tmp.part_next == -1)) {
            if (tmp.part_status != '0') {
                ebrs.push_back(tmp);
            }
            fseek(file, tmp.part_next, SEEK_SET);
            fread(&tmp, sizeof(Structs::EBR), 1, file);
        } else {
            fclose(file);
            break;
        }
    } while (true);
    return ebrs;
}

Structs::Partition Disk::findby(Structs::MBR mbr, string name, string path) {
    Structs::Partition partitions[4];
    partitions[0] = mbr.mbr_Partition_1;
    partitions[1] = mbr.mbr_Partition_2;
    partitions[2] = mbr.mbr_Partition_3;
    partitions[3] = mbr.mbr_Partition_4;

    bool ext = false;
    Structs::Partition extended;
    for (auto &partition : partitions) {
        if (partition.part_status == '1') {
            if (shared.compare(partition.part_name, name)) {
                return partition;
            } else if (partition.part_type == 'E') {
                ext = true;
                extended = partition;
            }
        }
    }
    if (ext) {
        vector<Structs::EBR> ebrs = getlogics(extended, path);
        for (Structs::EBR ebr : ebrs) {
            if (ebr.part_status == '1') {
                if (shared.compare(ebr.part_name, name)) {
                    Structs::Partition tmp;
                    tmp.part_status = '1';
                    tmp.part_type = 'L';
                    tmp.part_fit = ebr.part_fit;
                    tmp.part_start = ebr.part_start;
                    tmp.part_size = ebr.part_size;
                    strcpy(tmp.part_name, ebr.part_name);
                    return tmp;
                }
            }
        }
    }
    throw runtime_error("la partición no existe");
}

vector<Structs::Partition> Disk::getPartitions(Structs::MBR mbr){
    vector<Structs::Partition> partitions;
    partitions.push_back(mbr.mbr_Partition_1);
    partitions.push_back(mbr.mbr_Partition_2);
    partitions.push_back(mbr.mbr_Partition_3);
    partitions.push_back(mbr.mbr_Partition_4);
    return partitions;
}

Structs::MBR

Disk::adjust(Structs::MBR mbr, Structs::Partition p, vector<Transition> t, vector<Structs::Partition> ps, int u){
    if (u == 0) {
        p.part_start = sizeof(mbr);
        startValue = p.part_start;
        mbr.mbr_Partition_1 = p;
        return mbr;
    } else {
        Transition toUse;
        int c = 0;
        for (Transition tr : t) {
            if (c == 0) {
                toUse = tr;
                c++;
                continue;
            }

            if (toupper(mbr.disk_fit) == 'F') {
                if (toUse.before >= p.part_size || toUse.after >= p.part_size) {
                    break;
                }
                toUse = tr;
            } else if (toupper(mbr.disk_fit) == 'B') {
                if (toUse.before < p.part_size || toUse.after < p.part_size) {
                    toUse = tr;
                } else {
                    if (tr.before >= p.part_size || tr.after >= p.part_size) {
                        int b1 = toUse.before - p.part_size;
                        int a1 = toUse.after - p.part_size;
                        int b2 = tr.before - p.part_size;
                        int a2 = tr.after - p.part_size;

                        if ((b1 < b2 && b1 < a2) || (a1 < b2 && a1 < a2)) {
                            c++;
                            continue;
                        }
                        toUse = tr;
                    }
                }
            } else if (toupper(mbr.disk_fit) == 'W') {
                if (!(toUse.before >= p.part_size) || !(toUse.after >= p.part_size)) {
                    toUse = tr;
                } else {
                    if (tr.before >= p.part_size || tr.after >= p.part_size) {
                        int b1 = toUse.before - p.part_size;
                        int a1 = toUse.after - p.part_size;
                        int b2 = tr.before - p.part_size;
                        int a2 = tr.after - p.part_size;

                        if ((b1 > b2 && b1 > a2) || (a1 > b2 && a1 > a2)) {
                            c++;
                            continue;
                        }
                        toUse = tr;
                    }
                }
            }
            c++;
        }
        if (toUse.before >= p.part_size || toUse.after >= p.part_size) {
            if (toupper(mbr.disk_fit) == 'F') {
                if (toUse.before >= p.part_size) {
                    p.part_start = (toUse.start - toUse.before);
                    startValue = p.part_start;
                } else {
                    p.part_start = toUse.end;
                    startValue = p.part_start;
                }
            } else if (toupper(mbr.disk_fit) == 'B') {
                int b1 = toUse.before - p.part_size;
                int a1 = toUse.after - p.part_size;

                if ((toUse.before >= p.part_size && b1 < a1) || !(toUse.after >= p.part_start)) {
                    p.part_start = (toUse.start - toUse.before);
                    startValue = p.part_start;
                } else {
                    p.part_start = toUse.end;
                    startValue = p.part_start;
                }
            } else if (toupper(mbr.disk_fit) == 'W') {
                int b1 = toUse.before - p.part_size;
                int a1 = toUse.after - p.part_size;

                if ((toUse.before >= p.part_size && b1 > a1) || !(toUse.after >= p.part_start)) {
                    p.part_start = (toUse.start - toUse.before);
                    startValue = p.part_start;
                } else {
                    p.part_start = toUse.end;
                    startValue = p.part_start;
                }
            }
            Structs::Partition partitions[4];
            for (int i = 0; i < ps.size(); i++) {
                partitions[i] = ps.at(i);
            }
            for (auto &partition : partitions) {
                if (partition.part_status == '0') {
                    partition = p;
                    break;
                }
            }

            Structs::Partition aux;
            for (int i = 3; i >= 0; i--) {
                for (int j = 0; j < i; j++) {
                    if ((partitions[j].part_start > partitions[j + 1].part_start)) {
                        aux = partitions[j + 1];
                        partitions[j + 1] = partitions[j];
                        partitions[j] = aux;
                    }
                }
            }

            for (int i = 3; i >= 0; i--) {
                for (int j = 0; j < i; j++) {
                    if (partitions[j].part_status == '0') {
                        aux = partitions[j];
                        partitions[j] = partitions[j + 1];
                        partitions[j + 1] = aux;
                    }
                }
            }
            mbr.mbr_Partition_1 = partitions[0];
            mbr.mbr_Partition_2 = partitions[1];
            mbr.mbr_Partition_3 = partitions[2];
            mbr.mbr_Partition_4 = partitions[3];
            return mbr;
        } else {
            throw runtime_error("No hay espacio suficiente para esta particion");
        }
    }
}

void Disk::addpartition(string add, string u, string n, string p) {
    try {
        int add_int = stoi(add);
        if (shared.compare(u, "b") || shared.compare(u, "k") || shared.compare(u, "m")) {

            if (!shared.compare(u, "b")) {
                add_int *= (shared.compare(u, "k")) ? 1024 : 1024 * 1024;
            }
        } else {
            throw runtime_error("-u necesita valores específicos");
        }


        FILE *file = fopen(p.c_str(), "rb+");
        if (file == NULL) {
            throw runtime_error("disco no existente");
        }

        Structs::MBR disk;
        rewind(file);
        fread(&disk, sizeof(Structs::MBR), 1, file);

        findby(disk, n, p);

        Structs::Partition partitions[4];
        partitions[0] = disk.mbr_Partition_1;
        partitions[1] = disk.mbr_Partition_2;
        partitions[2] = disk.mbr_Partition_3;
        partitions[3] = disk.mbr_Partition_4;

        // cout<< partitions[0].part_start <<endl;
        // cout<< partitions[1].part_start <<endl;
        // cout<< partitions[2].part_start <<endl;
        // cout<< partitions[3].part_start <<endl;
        // cout<< disk.mbr_tamano <<endl;

        // cout<< partitions[0].part_size <<endl;
        // cout<< add_int <<endl;
        for (int i = 0; i < 4; i++) {
            if (partitions[i].part_status == '1') {
                if (shared.compare(partitions[i].part_name, n)) {
                    // cout<<"llego"<<endl;
                    // cout<<partitions[i].part_size + (add_int)<<endl;
                    // cout<<((partitions[i].part_size + (add_int)) > 0)<<endl;
                    // cout<<partitions[i].part_size<<endl;
                    if ((partitions[i].part_size + (add_int)) > 0) {
                        // cout<< partitions[i].part_size + (i) <<endl;
                        // cout<< partitions[i+1].part_start <<endl;
                                
                        if (i != 3) {
                            for (int j = 1; j < 4-i; j++)
                            {
                                if (partitions[i + j].part_status == '1'){
                                    if (partitions[i + j].part_start != 0) {
                                        if (((partitions[i].part_size + (add_int) +
                                            partitions[i].part_start) <=
                                            partitions[i + j].part_start)) {
                                            partitions[i].part_size += add_int;
                                            break;
                                        } else {
                                            throw runtime_error("se sobrepasa el límite ");
                                        }
                                    }
                                }

                            }
                            if ((partitions[i].part_size + add_int +
                            partitions[i].part_start) <= disk.mbr_tamano) {
                            partitions[i].part_size += add_int;
                            break;
                        } 
                        }
                        if ((partitions[i].part_size + add_int +
                            partitions[i].part_start) <= disk.mbr_tamano) {
                            partitions[i].part_size += add_int;
                            break;
                        } else {
                            throw runtime_error("se sobrepasa el límite 3");
                        }

                    } else {
                        throw runtime_error("add no valido ");
                    }
                }
            }
        }

        disk.mbr_Partition_1 = partitions[0];
        disk.mbr_Partition_2 = partitions[1];
        disk.mbr_Partition_3 = partitions[2];
        disk.mbr_Partition_4 = partitions[3];

        rewind(file);
        fwrite(&disk, sizeof(Structs::MBR), 1, file);
        shared.response("FDISK", "la partición se ha aumentado correctamente");
        fclose(file);
    }
    catch (exception &e) {
        shared.handler("FDISK", e.what());
        return;
    }

}

void Disk::logic(Structs::Partition partition, Structs::Partition ep, string p){
    Structs::EBR nlogic;
    nlogic.part_status = '1';
    nlogic.part_fit = partition.part_fit;
    nlogic.part_size = partition.part_size;
    nlogic.part_next = -1;
    strcpy(nlogic.part_name, partition.part_name);

    FILE *file = fopen(p.c_str(), "rb+");
    rewind(file);
    Structs::EBR tmp;
    fseek(file, ep.part_start, SEEK_SET);
    fread(&tmp, sizeof(Structs::EBR), 1, file);
    int size;
    do {
        size += sizeof(Structs::EBR) + tmp.part_size;
        if (tmp.part_status == '0' && tmp.part_next == -1) {
            nlogic.part_start = tmp.part_start;
            nlogic.part_next = nlogic.part_start + nlogic.part_size + sizeof(Structs::EBR);
            if ((ep.part_size - size) <= nlogic.part_size) {
                throw runtime_error("no hay espacio para más particiones lógicas");
            }
            fseek(file, nlogic.part_start, SEEK_SET);
            fwrite(&nlogic, sizeof(Structs::EBR), 1, file);
            fseek(file, nlogic.part_next, SEEK_SET);
            Structs::EBR addLogic;
            addLogic.part_status = '0';
            addLogic.part_next = -1;
            addLogic.part_start = nlogic.part_next;
            fseek(file, addLogic.part_start, SEEK_SET);
            fwrite(&addLogic, sizeof(Structs::EBR), 1, file);
            shared.response("FDISK", "partición creada correctamente");
            fclose(file);
            return;
        }
        fseek(file, tmp.part_next, SEEK_SET);
        fread(&tmp, sizeof(Structs::EBR), 1, file);
    } while (true);
}

void Disk::deletepartition(string d, string p, string n){
    try {

        if (p.substr(0, 1) == "\"") {
            p = p.substr(1, p.length() - 2);
        }

        if (!(shared.compare(d, "fast") || shared.compare(d, "full"))) {
            throw runtime_error("-delete necesita valores específicos");
        }

        FILE *file = fopen(p.c_str(), "rb+");
        if (file == NULL) {
            throw runtime_error("disco no existente");
        }

        Structs::MBR disk;
        rewind(file);
        fread(&disk, sizeof(Structs::MBR), 1, file);

        findby(disk, n, p);

        Structs::Partition partitions[4];
        partitions[0] = disk.mbr_Partition_1;
        partitions[1] = disk.mbr_Partition_2;
        partitions[2] = disk.mbr_Partition_3;
        partitions[3] = disk.mbr_Partition_4;

        Structs::Partition past;
        bool fll = false;
        for (int i = 0; i < 4; i++) {
            if (partitions[i].part_status == '1') {
                if (partitions[i].part_type == 'P') {
                    if (shared.compare(partitions[i].part_name, n)) {
                        if (shared.compare(d, "fast")) {
                            partitions[i].part_status = '0';
                        } else {
                            past = partitions[i];
                            partitions[i] = Structs::Partition();
                            fll = true;
                        }
                        break;
                    }
                } else {
                    if (shared.compare(partitions[i].part_name, n)) {
                        if (shared.compare(d, "fast")) {
                            partitions[i].part_status = '0';
                        } else {
                            past = partitions[i];
                            partitions[i] = Structs::Partition();
                            fll = true;
                        }
                        break;
                    }
                    vector<Structs::EBR> ebrs = getlogics(partitions[i], p);
                    int count = 0;
                    for (Structs::EBR ebr : ebrs) {
                        if (shared.compare(ebr.part_name, n)) {
                            ebr.part_status = '0';
                        }
                        fseek(file, ebr.part_start, SEEK_SET);
                        fwrite(&ebr, sizeof(Structs::EBR), 1, file);
                        count++;
                    }
                    shared.response("FDISK", "partición eliminada correctamente -" + d);
                    return;
                }
            }
        }

        Structs::Partition aux;
        for (int i = 3; i >= 0; i--) {
            for (int j = 0; j < i; j++) {
                if (partitions[j].part_status == '0') {
                    aux = partitions[j];
                    partitions[j] = partitions[j + 1];
                    partitions[j + 1] = aux;
                }
            }
        }

        disk.mbr_Partition_1 = partitions[0];
        disk.mbr_Partition_2 = partitions[1];
        disk.mbr_Partition_3 = partitions[2];
        disk.mbr_Partition_4 = partitions[3];

        rewind(file);
        fwrite(&disk, sizeof(Structs::MBR), 1, file);
        if (fll) {
            fseek(file, past.part_start, SEEK_SET);
            int num = static_cast<int>(past.part_size / 2);
            fwrite("\0", sizeof("\0"), num, file);
        }
        shared.response("FDISK", "partición eliminada correctamente -" + d);
        fclose(file);
    }
    catch (exception &e) {
        shared.handler("FDISK", e.what());
        return;
    }
}