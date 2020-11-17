# RD_2021

### FS
diretorias criadas:
-fsUsers, que continha depois todas as pastas e ficheiros relacionados com o fs
-*UID*, pasta dentro do fsUsers que é específica de cada user. É apagada quando o user é feito remove 
-*UID_fd.txt*, ficheiro que se encontra na *UID* e que contem informações como o socket associado ao User e a ação que pretende realizar no fs
-files, pasta contida em *UID*, que guarda os files associados a cada user, resultado dos comandos upload e retrieve

### AS

diretorias criadas:
-asUsers, que continha depois todas as pastas e ficheiros relacionados com o as
-*UID*, pasta dentro do asUsers que é específica de cada user. É apagada quando o user é feito remove 
-*UID_fd.txt*, ficheiro que se encontra na *UID* e que contem informações como o socket associado ao User 
-*UID_login.txt*, ficheiro que se encontra na *UID*, existente para sinalizar a existencia do User. Quando é fechada a sua sessão, é apagador
