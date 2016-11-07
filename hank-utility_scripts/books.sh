#!/bin/bash

ALL_BOOKS=~/Documents/books/all			# managed manually from books archieve
READ_BOOKS=~/Documents/books/done		# managed manually from read books and giving argument to the script
READING_BOOKS=~/Documents/books/persuit		# managed manually from interest
QUEUE=~/Documents/books/queue
desktop=~/Desktop

if [ $# -gt 0 ]
then
	if [ "$1" == "-r" ]
	then
		for books in "$@"
		do
			if [ "$books" == $1 ]
			then
				continue
			fi

			books=${books#$desktop}

			if [ -f "$READING_BOOKS/$books" ]
			then
				echo "moving $READING_BOOKS/$books to $READ_BOOKS/"
				mv "$READING_BOOKS/$books" $READ_BOOKS/
				rm "$desktop/$books"
			else
				echo "usage : books.sh [options]";
				echo "\"books.sh -h\" for more";
				exit
			fi
		done

	elif [ "$1" == "-h" ]
	then
		echo "usage : books.sh [options]";
		echo
		echo OPTIONS :
		printf "\t-h\t\t- this help menu\n"
		printf "\t-r [book_path]\t- add books to the read folder\n"
		exit

	else
		echo "usage : books.sh [options]";
		echo "\"books.sh -h\" for more";
		exit
	fi
fi

echo
for books_all in $ALL_BOOKS/* ;
do
	found=false
	books_all=${books_all#$ALL_BOOKS/}

	for books_read in $READ_BOOKS/* ;
	do
		books_read=${books_read#$READ_BOOKS/}

		if [ "$books_all" == "$books_read" ]
		then
			found=true
		fi
	done

	if [ $found == false ]
	then
		for books_persuit in $READING_BOOKS/*
		do
			books_persuit=${books_persuit#$READING_BOOKS/}

			if [ "$books_all" == "$books_persuit" ]
			then
				found=true
			fi
		done

		if [ $found == false ]
		then
			echo "copying $books_all to $QUEUE/"
			cp "$ALL_BOOKS/$books_all" $QUEUE/
		fi
	fi
done

echo
for copy_books in $READING_BOOKS/*
do
	echo "copying ${copy_books#$READING_BOOKS/} to $desktop/"
	cp "$copy_books" $desktop/
done
echo
echo '[+] all the books are sorted'
echo
