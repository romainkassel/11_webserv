NAME		=	webserv
CXX			=	c++
CXXFLAGS	=	-Wall -Werror -Wextra -std=c++98 -g -MMD -MP -I incs/
RM			=	rm -rf

SRCS_DIR	=	srcs/

CPP_FILES	=	main.cpp \
				ConfigParser.cpp ConfigLocation.cpp ConfigServer.cpp \
				HttpRequest.cpp HttpResponse.cpp ExecRequest.cpp \
				RunServer.cpp
SRCS		=	$(addprefix $(SRCS_DIR), $(CPP_FILES))
SRCS_OBJ	=	$(SRCS:.cpp=.o)

INCLUDE_PATH	=	incs/
INCLUDE_FILE	=	main.hpp \
					WebServ.hpp ConfigLocation.hpp ConfigServer.hpp \
					HttpRequest.hpp HttpResponse.hpp ExecRequest.hpp \
					RunServer.hpp
INCLUDE			=	$(addprefix $(INCLUDE_PATH), $(INCLUDE_FILE))

OBJS_DIR	=	.objs/
OBJS		=	$(addprefix $(OBJS_DIR), $(SRCS_OBJ))

D_FILES		=	$(addprefix $(OBJS_DIR), $(SRCS))

DEBUG = 0
ifeq ($(DEBUG), 1)
	CXXFLAGS += -g3
endif

all:	$(NAME)

$(NAME): $(OBJS) $(INCLUDE)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJS_DIR)%.o:	%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS_DIR)

fclean:	clean
	$(RM) $(NAME)

re:	fclean
	$(MAKE) all

sinclude $(D_FILES)

.PHONY:	all clean fclean re