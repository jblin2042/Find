#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>
char g_buf [ 8192 ] = { 0 } ;
int g_sz = 0 ;
void Printf ( const char* fmt , ... ) {
	va_list ap ;
	va_start ( ap , fmt ) ;
	int rtn = vsnprintf ( g_buf + g_sz , sizeof ( g_buf ) - g_sz , fmt , ap ) ;
	va_end ( ap ) ;
	g_sz += rtn ;
	if ( ( sizeof ( g_buf ) - 255 ) < g_sz ) {
		printf ( "%s" , g_buf ) ;
		g_buf [ 0 ] = 0 ;
		g_sz = 0 ;
	}
	return ;
}

typedef struct _S_dir {
	char path [ 1024 ] ;
	struct _S_dir * next ;
} S_dir ;

// 取出後記得自行 free node //
S_dir* Pop ( S_dir** head ) {
	S_dir* p_tmp = * head ;
	if ( * head ) {
		* head = ( * head )->next ;
	}
	return p_tmp ;
}

int Push ( S_dir** head , char* dir_path ) {
	S_dir* p_new_node = ( S_dir* ) calloc ( 1 , sizeof(S_dir) ) ;
	if ( ! p_new_node ) {
		return - 1 ;
	}
	snprintf ( p_new_node->path , sizeof ( p_new_node->path ) , "%s" , dir_path ) ;
	p_new_node->next = * head ;
	* head = p_new_node ;
	return 1 ;
}

void Show_list ( S_dir * head ) {
	while ( head ) {
		printf ( "%s\n" , head->path ) ;
		head = head->next ;
	}
	return ;
}

void Find_recursion ( char* dir_path ) {
	DIR* dir = opendir ( dir_path ) ;
	if ( ! dir ) {
		return ;
	}
	char file_path [ 1024 ] = { 0 } ;
	struct dirent * rd_dir = NULL ;
	while ( NULL != ( rd_dir = readdir ( dir ) ) ) {
		if ( strcmp ( rd_dir->d_name , "." ) && strcmp ( rd_dir->d_name , ".." ) ) {
			Printf ( "%s%s\n" , dir_path , rd_dir->d_name ) ;
			if ( DT_DIR == rd_dir->d_type ) {
				snprintf ( file_path , sizeof ( file_path ) , "%s%s/" , dir_path , rd_dir->d_name ) ;
				Find_recursion ( file_path ) ;
				continue ;
			}
		}
	}
	closedir ( dir ) ;
	return ;
}

int Find_narmal ( char* dir_path ) {
	char file_path [ 1024 ] = { 0 } ;
	S_dir *head = NULL , *p_pop = NULL ;
	Push ( & head , dir_path ) ;

	while ( head ) {
		p_pop = Pop ( & head ) ;
		DIR* dir = opendir ( p_pop->path ) ;
		if ( ! dir ) {
			free ( p_pop ) ; p_pop = NULL ;
			return - 1 ;
		}
		struct dirent * rd_dir = NULL ;
		while ( NULL != ( rd_dir = readdir ( dir ) ) ) {
			if ( strcmp ( rd_dir->d_name , "." ) && strcmp ( rd_dir->d_name , ".." ) ) {
				Printf ( "%s%s\n" , p_pop->path , rd_dir->d_name ) ;
				if ( DT_DIR == rd_dir->d_type ) {
					snprintf ( file_path , sizeof ( file_path ) , "%s%s/" , p_pop->path , rd_dir->d_name ) ;
					Push ( & head , file_path ) ;
				}
			}
		}
		closedir ( dir ) ;
		free ( p_pop ) ; p_pop = NULL ;
	}
	return 1 ;
}

struct timeval g_timer_begin ;
struct timeval g_timer_end ;
void Begin_time ( void ) {
	gettimeofday ( & g_timer_begin , NULL ) ;
	return ;
}

long double End_time ( void ) {
	gettimeofday ( & g_timer_end , NULL ) ;
	long int d_s = g_timer_end.tv_sec - g_timer_begin.tv_sec ;
	long int d_us = g_timer_end.tv_usec - g_timer_begin.tv_usec ;
	if ( g_timer_end.tv_usec < g_timer_begin.tv_usec ) {
		-- d_s ;
		d_us = ( 1000000 - g_timer_begin.tv_usec ) + g_timer_end.tv_usec ;
	}
	return d_s + ( ( long double ) d_us / 10e5 ) ;
}

int main ( int argc , char*argv [ ] ) {

	if ( 2 > argc ) {
		return - 1 ;
	}
	char buf [ 1024 ] = { 0 } ;
	snprintf ( buf , sizeof ( buf ) , "%s" , argv [ 1 ] ) ;
	if ( '/' != buf [ strlen ( buf ) - 1 ] ) {
		strcat ( buf , "/" ) ;
	}

	system ( "echo 3 > /proc/sys/vm/drop_caches" ) ;
	// 執行程式的時候多給一個參數 ,會執行 find //

	int opt = 0 ;
	if ( 2 < argc ) {
		opt = atoi ( argv [ 2 ] ) ;
	}
	char cmmd_buf [ 1024 ] = { 0 } ;
	snprintf ( cmmd_buf , sizeof ( cmmd_buf ) , "find %s" , buf ) ;
	Begin_time ( ) ;
	switch ( opt ) {
		case 1 : {
			system ( cmmd_buf ) ;
			snprintf ( cmmd_buf , sizeof ( cmmd_buf ) , "<system>" ) ;
		}
			break ;

		case 2 :
			Find_recursion ( buf ) ;
			snprintf ( cmmd_buf , sizeof ( cmmd_buf ) , "<my find recursion>" ) ;

			break ;

		case 3 :
			Find_narmal ( buf ) ;
			snprintf ( cmmd_buf , sizeof ( cmmd_buf ) , "<my find normal>" ) ;
			break ;
	}
	printf ( "%s Diff time:%Lf\n" , cmmd_buf , End_time ( ) ) ;

	return 1 ;
}
